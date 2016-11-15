// ***************************************************************
// ** CLASS: Camface
// ***************************************************************

function Camface(args){
	var self = this;
	
	// Check for getUserMedia support
	navigator.getUserMedia = navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia || navigator.msGetUserMedia || navigator.oGetUserMedia;
	if(!navigator.getUserMedia){
		console.log('ERROR: No support for WebRTC webcam capture!');
		return;
	}
	
	// Check arguments (and set to default if unspecified)
	if(arguments.length == 0) args = {};
	this.width = ('width' in args) ? args['width'] : 640;
	this.height = ('height' in args) ? args['height'] : 480;
	this.callback = ('callback' in args) ? args['callback'] : undefined;
	this.source = ('source' in args) ? args['source'] : undefined;
	
	// Prepare video source
	if(this.source == undefined){
		this._video = ('video' in args) ? args['video'] : document.createElement('video');
		this._video.autoplay = true;
		this._video.oncanplay = function(){ self._processFirstFrame(); };
	}
	// Prepare canvas
	this._canvas = ('canvas' in args) ? args['canvas'] : document.createElement('canvas');
	this._canvas.width = this.width;
	this._canvas.height = this.height;

	// Prepare context and asm.js buffer
	this._stream = undefined;
	this._context = this._canvas.getContext('2d');
	this._buffer = this._captureBuffer();

	// Prepare history of detected faces
	this._history_faces = [];

	return this;
}

// ** LIBRARY MANAGEMENT CALLS
// *******************************************

Camface.prototype.start = function(){
	// Process a single frame if source given at build time
	if(this.source != undefined){
		this._processFirstFrame();
		return;
	}
	
	// Start capturing video
	var self = this;
	navigator.getUserMedia(
		{video: true, mandatory: {width: this.width, height: this.height}},
		function(s){ self._mediaCaptureSuccess(s); },
		function(e){ self._mediaCaptureError(e) }
	);
};

Camface.prototype.stop = function(){
	if(this.source != undefined || !this._stream) return;
	this._video.pause();
	this._stream.getTracks()[0].stop();
	this._stream = undefined;
};

Camface.prototype.setCallback = function(callback){
	this.callback = callback;
};

// ** PIPELINE FOR THE LIBRARY
// *******************************************

// Process initial frame (detect face and expressions)
Camface.prototype._processFirstFrame = function(){
	if(!this._captureData()) return false;
	
	// Detect face and facial expressions, callback
	this.bounding_box = this._detectFace();
	this.expressions  = this._recognizeExpression();
	this._fillCanvasWithArray(this._buffer); // TODO: Bullshit, only used to test input
	if(this.callback != undefined) this.callback(this.bounding_box, this.expressions);
	
	// Schedule next execution
	var self = this;
	setTimeout(function(){
		if(self.source == undefined) self._processFrame();
	}, 100);
};

// Process a frame (track face and detect expressions)
Camface.prototype._processFrame = function(){
	if(!this._captureData()) return false;
	
	// Track face and facial expressions, callback
	this.bounding_box = this._trackFace();
	this.expressions  = this._recognizeExpression();
	this._fillCanvasWithArray(this._buffer); // TODO: Bullshit, only used to test input
	if(this.callback != undefined) this.callback(this.bounding_box, this.expressions);

	// Schedule next execution
	var self = this;
	setTimeout(function(){ self._processFrame(); }, 100);
};

Camface.prototype._fillCanvasWithArray = function(arr){
	var imgData = this._context.getImageData(0, 0, this.width, this.height);
	var data = imgData.data;
	data.set(arr);
	this._context.putImageData(imgData, 0, 0);
};

// ** CALLS TO ASM.JS
// *******************************************

// Capture image buffer from asm.js library
Camface.prototype._captureBuffer = function(){
	var ptr = _capture_buffer(this.width, this.height);
	return Module.HEAPU8.subarray(ptr, ptr + 4*this.width*this.height);
};

// Capture a camera frame and copy to asm.js image buffer
Camface.prototype._captureData = function(){
	// Capture frame data
	if((this.source == undefined) && (this._video.paused || this._video.ended)) return false;
	this._context.drawImage((this.source == undefined) ? this._video : this.source, 0, 0, this.width, this.height);
	var data = this._context.getImageData(0, 0, this.width, this.height).data;
	
	// Copy to asm.js buffer
	this._buffer.set(data);
	return true;
};

// Detect face
Camface.prototype._detectFace = function(){
	var i;

	var detections = ccv.detect_objects({
		"canvas" : ccv.grayscale(this._canvas),
		"cascade" : cascade,
		"interval" : 5,
		"min_neighbors" : 1
	});

	// Discard regions below threshold size
	var threshold = this.width / 10;
	for(i=detections.length-1;i>=0;i--){
		if(detections[i].width < threshold) detections.splice(i, 1);
	}

	// If no face found, return whole image
	if(detections.length == 0) return this._recover_missing();

	// Find highest confidence detection
	var best = detections[0];
	for(i=0;i<detections.length;i++){
		if(detections[i].confidence > best.confidence) best = detections[i];
	}

	var ret = [best.x-0.075*best.width, best.y+0.04*best.height, 1.15 * best.width, 1.15 * best.height];
	this._history_faces.push(ret);
	if(this._history_faces.length > 100) this._history_faces.splice(0, 1);
	return ret;
};

// Track face (knowing its previous location)
Camface.prototype._trackFace = function(){
	return this._detectFace();
};

// Recognize facial expression
// Internally (asm.js) it crops and resizes the facial region before processing
Camface.prototype._recognizeExpression = function(){
	var ptr = _recognize_expression();
	return Module.HEAPF32.subarray(ptr>>4, (ptr>>4)+7);
};

// ** WEBCAM ADQUISITION CALLBACKS
// *******************************************

// If success acquiring the web-cam
Camface.prototype._mediaCaptureSuccess = function(stream){
	this._stream = stream;
	this._video.src = window.URL.createObjectURL(stream);
};

// If failed to acquire the web-cam
Camface.prototype._mediaCaptureError = function(e){
	console.log('ERROR: Failed to get user media!');
};

// ** POST-PROCESSING FUNCTIONS
// *******************************************

Camface.prototype._recover_missing = function(){
	if(this._history_faces.length == 0)	return [0, 0, this.width, this.height];
	return this._history_faces[this._history_faces.length-1]
}