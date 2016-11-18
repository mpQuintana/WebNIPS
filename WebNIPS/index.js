
var server = require('./server'),
    handlers = require('./handlers'),
    router = require('./router'),
    handle = { };

handle["/"] = handlers.home;
handle["/home"] = handlers.home;
handle["/upload"] = handlers.upload;
handle["/sendMail"] = handlers.sendMail;
handle._static = handlers.serveStatic;

server.start(router.route, handle);
