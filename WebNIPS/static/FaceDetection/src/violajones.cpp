#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cmath>
#include "../inc/haar.h"
#include "../inc/violajones.h"
#include "../inc/connected.h"


// ***************************************************************
// ** PUBLIC CLASS METHODS
// ***************************************************************

ViolaJones::ViolaJones(int w, int h){
    this->imW = w;
    this->imH = h;

    // Pre-allocate integral image
    this->integralImage = (float *)new float[w*h];
    this->sqIntegralImage = (float *)new float[w*h];

    // Load cascade
    this->cascade = Cascade(HAAR_WIDTH, HAAR_HEIGHT, haar_data1);
}

ViolaJones::~ViolaJones()
{
    // Free integral image memory
    delete this->integralImage;
    delete this->sqIntegralImage;
}

/** ViolaJones::detect
  * Detect face given image
  * image: pointer to the image array
  * rect: 4-element array where the face location is to be placed
  **/
Rect ViolaJones::detect(unsigned char* image, unsigned short* rect)
{
    // Compute integral images
    this->generateIntegralImage(image);
    this->generateSquareIntegralImage(image);

    // Define detector
    double scale = 1.1;
    double shift = 0.02;
    unsigned int refWSz = 260;
    Detector detector(scale, shift, refWSz);

    // Apply to whole frame
    Frame frame(this->integralImage, this->sqIntegralImage, this->imW, this->imH);
    Rect wholeFrame(0,0,frame.getWidth(), frame.getHeight());
    Rect detection = detector.apply(this->cascade, wholeFrame, frame);

    // Fill positive
    //float s = 0.75;
    //float dif = (1-s)*detection.getWidth();

    rect[0] = detection.getX();
    rect[1] = detection.getY();
    rect[2] = detection.getWidth();
    rect[3] = detection.getHeight();

    return detection;
}

/** ViolaJones::track
  * Track face given previous location and new image
  * image: pointer to the image array
  * rect: 4-element array where the new face location is to be placed
  **/
Rect ViolaJones::track(unsigned char* image, unsigned short* roi, unsigned short* rect){

    this->generateIntegralImage(image);

    // Compute integral images
    this->generateIntegralImage(image);
    this->generateSquareIntegralImage(image);

    // Define detector
    double scale = 1.1;
    double shift = 0.02;
    unsigned int refWSz = 260;
    Detector detector(scale, shift, refWSz);

    // Apply to whole frame
    Frame frame(this->integralImage, this->sqIntegralImage, this->imW, this->imH);
    Rect roiFrame(roi[0],roi[1],roi[2],roi[3]);
    Rect detection = detector.apply(cascade, roiFrame, frame);

    // Fill positive
    rect[0] = detection.getX();
    rect[1] = detection.getY();
    rect[2] = detection.getWidth();
    rect[3] = detection.getHeight();

    return detection;
}

// ***************************************************************
// ** PRIVATE CLASS METHODS
// ***************************************************************

void ViolaJones::generateIntegralImage(unsigned char* image){
    float* intIm = this->integralImage;

    // Define norm (8b, 3channels)
    //const int NORM = 255*3;
    const int NORM = 3;


    // Fill first pixel
    intIm[0] = (float)(image[0] + image[1] + image[2])/NORM;

    // Fill first row
    for(int i=1;i<this->imW;++i)
    {
        // Indices
        int pos = this->nChn*i;
        int idx = i;

        // Integral values
        intIm[idx] = ((float)(image[pos] + image[pos+1] + image[pos+2])) / NORM + intIm[idx-1];
    }

    // Fill first column
    for(int j=1;j<this->imH;++j)
    {
        // Indices
        int pos = this->nChn*(j*this->imW);
        int idx = j*this->imW;
        int idxPrev = (j-1)*this->imW;

        // Integral value
        intIm[idx] = ((float)(image[pos] + image[pos+1] + image[pos+2])) / NORM + intIm[idxPrev];
    }


    // Fill rest of matrix
    for(int i=1;i<this->imW;++i)
    {
        for(int j=1;j<this->imH;++j)
        {
            // Indices
            int pos = this->nChn*(i+j*this->imW);
            int idx = (i+j*this->imW);
            int idxUp = i+(j-1)*this->imW;
            int idxLeft = i-1 + j*this->imW;
            int idxUpLeft = i-1 + (j-1)*this->imW;

            // Value at current position
            float curVal = ((float)(image[pos] + image[pos+1] + image[pos+2])) / NORM;

            // Integral value
            intIm[idx] = curVal + intIm[idxUp] + intIm[idxLeft] - intIm[idxUpLeft];
        }
    }
}

void ViolaJones::generateSquareIntegralImage(unsigned char* image){
    float* intIm = this->sqIntegralImage;

    // Define norm (8b, 3channels)
    //const int NORM = 255*255*3;
    const int NORM = 3;

    // Fill first pixel
    float grey = (float)(image[0] + image[1] + image[2])/NORM;
    intIm[0] = (float)grey*grey;

    // Fill first row
    for(int i=1;i<this->imW;++i)
    {
        // Indices
        int pos = this->nChn*i;
        int idx = i;

        // Integral values
        grey = (image[pos] + image[pos+1] + image[pos+2])/NORM;
        intIm[idx] = (float)(grey*grey) + intIm[idx-1];
    }

    // Fill first column
    for(int j=1;j<this->imH;++j)
    {
        // Indices
        int pos = this->nChn*(j*this->imW);
        int idx = j*this->imW;
        int idxPrev = (j-1)*this->imW;

        // Integral value
        grey = (image[pos] + image[pos+1] + image[pos+2])/NORM;
        intIm[idx] = (float)(grey*grey) + intIm[idxPrev];
    }


    // Fill rest of matrix
    for(int i=1;i<this->imW;++i)
    {
        for(int j=1;j<this->imH;++j)
        {
            // Indices
            int pos = this->nChn*(i+j*this->imW);
            int idx = (i+j*this->imW);
            int idxUp = i+(j-1)*this->imW;
            int idxLeft = i-1 + j*this->imW;
            int idxUpLeft = i-1 + (j-1)*this->imW;

            // Value at current position
            grey = (image[pos] + image[pos+1] + image[pos+2])/NORM;
            float curVal = (float)(grey*grey);

            // Integral value
            intIm[idx] = curVal + intIm[idxUp] + intIm[idxLeft] - intIm[idxUpLeft];
        }
    }
}

void Rect::set(int x, int y, int width, int height)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
}

void Rect::shift(int dX, int dY)
{
    this->x += dX;
    this->y += dY;
}

void Rect::scale(double sX, double sY)
{
    this->x = static_cast<int>(this->x*sX);
    this->y = static_cast<int>(this->y*sY);
    this->width = static_cast<int>(this->width*sX);
    this->height = static_cast<int>(this->height*sY);
}

void Rect::origScale(double sX, double sY)
{
    this->width = static_cast<int>(this->width*sX);
    this->height = static_cast<int>(this->height*sY);
}

void Rect::centerScale(double sX, double sY)
{
    int scaledWidth = static_cast<int>(this->width*sX);
    int scaledHeight = static_cast<int>(this->height*sY);

    int dx = this->width - static_cast<int>(this->width*sX);
    int dy = this->height - static_cast<int>(this->height*sY);

    this->x -= static_cast<int>(dx/2);
    this->y -= static_cast<int>(dy/2);

    this->width = scaledWidth;
    this->height = scaledHeight;
}

bool Rect::isInside(Rect& r)
{
    // Compute center coordinates
    double centerX = this->getX() + this->getWidth()/2;
    double centerY = this->getY() + this->getHeight()/2;

    if(centerX>r.getX() && centerX<(r.getX() + r.getWidth())&&
       centerY>r.getY() && centerY<(r.getY()) + r.getHeight())
        return true;
    else
        return false;
}

void Rect::limitTo(Rect& r)
{
    if(this->getX()<r.getX())
        this->setX(r.getX());

    if(this->getY()<r.getY())
        this->setY(r.getY());

    if(this->getX()+this->getWidth()>r.getX()+r.getWidth())
        this->setWidth(r.getX()+r.getWidth()-this->getX());

    if(this->getY()+this->getHeight()>r.getY()+r.getHeight())
        this->setHeight(r.getY()+r.getHeight()-this->getX());
}

std::vector<Rect> Stage::apply(std::vector<Rect> windows, Frame& im)
{
    std::vector<Rect> positives;

    // For every window
    for(std::vector<Rect>::iterator window = windows.begin(); window!=windows.end(); ++window)
    {
        // Window mean
        double winMean = im.sumOver(*window)/window->area();

        // Standard deviation
        double winStdDev = im.stdDevOver(*window);
        //double winStdDev = 2;

        if(winStdDev>1)
        {
            // Extract features
            double val = 0;
            for(std::vector<Feature>::iterator feature=this->feat.begin(); feature!=this->feat.end(); ++feature)
            {
                val += feature->extract(*window, im, winMean, winStdDev);
            }

            // If negative discard window
            if(val<this->T)
                positives.push_back(*window);
        }
    }

    return positives;
}

// Sort labels according to cardinality
struct Sorter
{
  bool operator()(std::pair<int, std::vector<std::pair<int,int> > > a,
                std::pair<int, std::vector<std::pair<int,int> > > b)
  {
       return a.first > b.first;
  }
}labelSorter;

std::pair<Rect,double> Merger::apply(Frame& frame, std::vector<Rect>& rects, unsigned int shift)
{
    // refer to "An Analisys of the Viola-Jones Face Detection Algorithm", Yi-Qing Wang (Algorithm 11)

    if(!rects.empty())
    {
        // 1. Create a matrix of the same size as frame.
        int ssmpl_width = static_cast<int>(frame.getWidth()/shift);
        int ssmpl_height = static_cast<int>(frame.getHeight()/shift);

        unsigned int *in = new unsigned int[ssmpl_width*ssmpl_height]();
        unsigned int *out = new unsigned int[ssmpl_width*ssmpl_height]();

        // 2. Fill matrix
        for(std::vector<Rect>::iterator r = rects.begin(); r!=rects.end(); ++r)
        {
            unsigned int pos = (r->getY()*ssmpl_width + r->getX())/shift;
            in[pos] = 1;
        }

        // 3. Run connected component labeling algorithm
        const unsigned int N_LABELS = 100; //Estimation
        ConnectedComponents cc(N_LABELS);
        cc.connected(in, out, ssmpl_width, ssmpl_height, std::equal_to<unsigned char>(),false);

        int nLabels = cc.getHighestLabel();

        std::vector<std::pair<int, std::vector<std::pair<int,int> > > > clusters(nLabels);
        for(int pos = 0; pos<ssmpl_width*ssmpl_height; pos++)
        {
            int y = static_cast<int>(pos/ssmpl_width);
            int x = pos - y*ssmpl_width;

            clusters[out[pos]].first++;
            clusters[out[pos]].second.push_back(std::pair<int,int>(x,y));
        }

        std::sort(clusters.begin(), clusters.end(), labelSorter);

        // Remove background
        clusters.erase(clusters.begin());

        // Return highest confidence cluster
        std::pair<int, int> origin = this->getClusterRepresentant(clusters.front().second);
        Rect detection(origin.first*shift, origin.second*shift, rects[0].getWidth(), rects[0].getHeight());

        delete in;
        delete out;

        if (clusters.front().first > this->confidence)
            return std::pair<Rect, double>(detection, clusters.front().first);
        else
            return std::pair<Rect, double>(Rect(), 0);
    }
    else
        return std::pair<Rect, double>(Rect(), 0);
}

std::pair<int, int> Merger::getClusterRepresentant(std::vector<std::pair<int, int> >& in)
{
    std::vector<int> x;
    std::vector<int> y;

    unsigned int acc_x = 0;
    unsigned int acc_y = 0;

    for(unsigned int i=0; i<in.size(); ++i)
    {
        acc_x += in[i].first;
        acc_y += in[i].second;
    }

    return std::pair<int, int>(acc_x/in.size(), acc_y/in.size());
}

double Frame::get(unsigned int x, unsigned int y)
{
    return this->data[x + y*this->width];
}

double Frame::getSq(unsigned int x, unsigned int y)
{
    return this->sqData[x + y*this->width];
}

// Get sum over Rect
double Frame::sumOver(Rect& r)
{
    // Get values
    double curVal = this->get(r.getX()+r.getWidth(), r.getY()+r.getHeight());
    double upVal = this->get(r.getX()+r.getWidth(), r.getY());
    double leftVal = this->get(r.getX(), r.getY()+r.getHeight());
    double diagVal = this->get(r.getX(), r.getY());

    // Sum
    return (curVal - upVal - leftVal + diagVal);
}

// Extract mean and get sum over Rect
double Frame::sumOver(Rect& r, double mean, double stdDev)
{
    // Get values
    double curVal = this->get(r.getX()+r.getWidth(), r.getY()+r.getHeight());
    double upVal = this->get(r.getX()+r.getWidth(), r.getY());
    double leftVal = this->get(r.getX(), r.getY()+r.getHeight());
    double diagVal = this->get(r.getX(), r.getY());

    // Sum
    return (curVal - upVal - leftVal + diagVal)/stdDev - mean*r.getHeight()*r.getWidth();
}

// Get sum over Rect
double Frame::sumOverSq(Rect& r)
{
    // Get values
    double curVal = this->getSq(r.getX()+r.getWidth(), r.getY()+r.getHeight());
    double upVal = this->getSq(r.getX()+r.getWidth(), r.getY());
    double leftVal = this->getSq(r.getX(), r.getY()+r.getHeight());
    double diagVal = this->getSq(r.getX(), r.getY());

    // Sum
    return (curVal - upVal - leftVal + diagVal);
}

// Extract mean and get sum over Rect
double Frame::sumOverSq(Rect& r, double mean)
{
    // Get values
    double curVal = this->getSq(r.getX()+r.getWidth(), r.getY()+r.getHeight());
    double upVal = this->getSq(r.getX()+r.getWidth(), r.getY());
    double leftVal = this->getSq(r.getX(), r.getY()+r.getHeight());
    double diagVal = this->getSq(r.getX(), r.getY());

    // Sum
    return (curVal - upVal - leftVal + diagVal) - mean*r.getHeight()*r.getWidth();
}

double Frame::stdDevOver(Rect& win)
{
    double mean = sumOver(win)/win.area();

    double S1 = this->sumOver(win);
    double S2 = this->sumOverSq(win);
    unsigned int n = win.getWidth()*win.getHeight();

    return sqrt((S2-S1*S1/n)/n);
}

double Feature::extract(Rect& win, Frame& im, double mean, double stdDev)
{
    double out = 0;

    // Compute scale
    double scaleX = win.getWidth()/this->refSize.first;
    double scaleY = win.getHeight()/this->refSize.second;

    double val = 0;
    for(std::vector<std::pair<Rect,double> >::iterator r = this->rect.begin(); r!=this->rect.end(); ++r)
    {
        // Scale to window
        Rect feat_w = r->first;
        feat_w.scale(scaleX, scaleY);
        double weight = r->second;

        // Compute coordinates
        unsigned int origX = win.getX() + feat_w.getX();
        unsigned int origY = win.getY() + feat_w.getY();
        unsigned int width = feat_w.getWidth();
        unsigned int height = feat_w.getHeight();

        // Get value
        Rect scaled_win(origX, origY, width, height);

        val += weight*im.sumOver(scaled_win, mean, stdDev);
    }

    if(val>this->T)
        out += this->lVal;
    else
        out += this->rVal;

    return out;
}


// Sort labels according to cardinality
struct SizeSorter
{
  bool operator()(std::pair<Rect, double>& a, std::pair<Rect, double>& b)
  {
       return a.first.getWidth()<b.first.getWidth();
  }
}szSorter;

// Sort in ascending order of confidence
struct ConfSorter
{
  bool operator()(std::pair<Rect, double>& a, std::pair<Rect, double>& b)
  {
       return a.second>b.second;
  }
}cfSorter;



Rect Detector::apply(Cascade& cascade, Rect& roi, Frame& im)
{
    std::vector<std::pair<Rect,double> > positives;

    const int N_SCALES = 5;
    double scale = 1;

    // Define merger
    const double CONFIDENCE = 10;
    Merger merger(CONFIDENCE);

    for (int sc=1; sc<=N_SCALES; ++sc)
    {   
        // Define window to shift
        Rect refWin(0,0,this->wSize*scale,this->wSize*scale);

        // Shift in pixels (relative to window size)
        unsigned int pix_shift = static_cast<int>(this->shift*this->wSize*scale);

        // Generate set of shifted windows in roi
        std::vector<Rect> windows = generateWindows(roi, refWin);

        // Find positives
        std::vector<Rect> out = this->step(cascade, windows, im);

        // Define post-processing
        std::pair<Rect,double> pstv = merger.apply(im, out, pix_shift);

        // Append to output
        positives.push_back(pstv);

        // Increment scale
        scale = scale*this->scale;
    }

    // Sort the representing windows in ascending order of window size (square wins assumed)
    std::sort (positives.begin(), positives.end(), szSorter);

    // Filter positives
    // If the smaller window's center is outside the bigger one, keep both
    // Keep the one with higher detection otherwise
    for(std::vector<std::pair<Rect, double> >::iterator win = positives.begin(); win!=positives.end(); ++win)
    {
        for(std::vector<std::pair<Rect, double> >::iterator bigger_win = win+1; bigger_win!=positives.end(); ++bigger_win)
        {
            //std::cout<<win->second<<std::endl;
            //std::cout<<bigger_win->second<<std::endl;

            if(win->first.isInside(bigger_win->first))
            {
                if(win->second>=bigger_win->second)
                {
                    positives.erase(bigger_win);
                    --bigger_win;
                }
                else
                {
                    positives.erase(win);
                    --win;
                    break;
                }
            }
        }
    }

    std::sort (positives.begin(), positives.end(), cfSorter);

    // 8. Return highest confidence positive
    return positives.begin()->first;
}

std::vector<Rect> Detector::step(Cascade& cascade, std::vector<Rect> windows, Frame& im)
{
    // refer to "An Analisys of the Viola-Jones Face Detection Algorithm", Yi-Qing Wang (Algorithm 7)
    // refer to "Rapid Object detection using Boosted Cascade of Simple Featuer", P. Viola, M. Jones

    // Start with all windows
    std::vector<Rect> positives = windows;

    // Every layer of the cascade
    for(std::vector<Stage>::iterator lyr = cascade.layer.begin(); lyr != cascade.layer.end(); ++lyr)
    {

        // Get positives for current layer
        if (!positives.empty())
            positives = lyr->apply(positives, im);
    }

    // Return positive windows
    return positives;
}

std::vector<Rect> Detector::generateWindows(Rect& roi, Rect& win)
{
   std::vector<Rect> windows;

   if(win.getWidth()<roi.getWidth() && win.getHeight()<roi.getWidth())
   {
       // Set window to ROI origin
       win.setX(roi.getX());
       win.setY(roi.getY());

       // Shift in pixels (relative to window size)
       double deltaX = static_cast<int>(this->shift*win.getWidth());
       double deltaY = static_cast<int>(this->shift*win.getHeight());

       while(win.getY()+win.getHeight()<roi.getY()+roi.getHeight())
       {
           while(win.getX()+win.getWidth()<roi.getX()+roi.getWidth())
           {
               windows.push_back(win);
               win.shift(deltaX,0);//Shift to right
           }

           // Next row
           win.setX(roi.getX());
           win.shift(0,deltaY);//Next row
       }
   }
   return windows;
}

Cascade::Cascade(std::string& path2File)
{
    std::vector<Stage> cascade;

    std::string sLine = "";
    std::ifstream infile;

    infile.open(path2File);

    unsigned int ln = 0;

    // Init feature vector
    std::vector<Feature> features;
    int currentStage = 0;
    double stageT = 0;

    while (std::getline(infile, sLine))
    {
        std::vector<double> params = split(sLine, ' ');

        if(ln>0)
        {
            // New stage
            if (params[0]==(currentStage+1))
            {
                // Load stage into cascade
                Stage stage(features,stageT);
                this->layer.push_back(stage);

                // Clear features
                features.clear();
                currentStage++;
            }

            //std::transform(tokens.begin(), tokens.end(), params.begin(), std::stod);

            // Load rectangles
            std::vector<std::pair<Rect,double> > rects;
            for (unsigned int i=5;i<params.size();i+=5)
            {
                Rect r(params[i],params[i+1], params[i+2], params[i+3]);

                double weight = params[i+4];
                rects.push_back(std::pair<Rect, double>(r,weight));
            }

            // Load feature
            std::pair<unsigned int, unsigned int> refSize (this->sizeW, this->sizeW);
            Feature f(rects, params[2], params[3], params[4], refSize);
            features.push_back(f);

            // Load stage threshold
            stageT = params[1];
        }
        else
        {
            // Load cascade reference window size
            this->sizeW = params[0];
            this->sizeH = params[1];
        }

        ln++;
    }

    // Load last stage into cascade
    Stage stage(features,stageT);
    this->layer.push_back(stage);

}

Cascade::Cascade(const int W, const int H, double data[2912][20])
{
    std::vector<Stage> cascade;

    // Init feature vector
    std::vector<Feature> features;
    int currentStage = 0;
    double stageT = 0;

    // Load reference size
    this->sizeW = W;
    this->sizeH = H;

    // Load cascade
    for(unsigned int i = 0; i<=2912; ++i)
    {
        // New stage
        if (data[i][0]==(currentStage+1))
        {
            // Load stage into cascade
            Stage stage(features,stageT);
            this->layer.push_back(stage);

            // Clear features
            features.clear();
            currentStage++;
        }

        // Load rectangles
        std::vector<std::pair<Rect,double> > rects;
        for (unsigned int k=5;k<20;k+=5)
        {
            if (data[i][k+4]!=0)
            {
                Rect r(data[i][k], data[i][k+1], data[i][k+2], data[i][k+3]);

                double weight = data[i][k+4];
                rects.push_back(std::pair<Rect, double>(r,weight));
            }
        }

        // Load feature
        std::pair<unsigned int, unsigned int> refSize (this->sizeW, this->sizeW);
        Feature f(rects, data[i][2], data[i][3], data[i][4], refSize);
        features.push_back(f);

        // Load stage threshold
        stageT = data[i][1];
    }

    // Load last stage into cascade
    Stage stage(features,stageT);
    this->layer.push_back(stage);
}

std::vector<double> split(std::string str, char delimiter)
{
  std::vector<double> out;
  std::stringstream ss(str); // Turn the string into a stream.
  std::string tok;

  while(std::getline(ss, tok, delimiter))
  {
    out.push_back(std::stod(tok));
  }

  return out;
}

