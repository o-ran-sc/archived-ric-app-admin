import cherrypy
import json
import os;

Rates = {};

@cherrypy.expose
class VES(object):
    def __init__ (self):
        self._name = "test";

    @cherrypy.tools.accept(media='application/json')
    def POST (self):
        data = cherrypy.request.body.read(int(cherrypy.request.headers['Content-Length']));
        json_data = json.loads(data);
        measurement_interval = -1;
        sgnb_req_count = 0;
        sgnb_rej_count = 0;
        class_id = None;
        timestamp = 0;
 
        if 'event' in json_data:
            if  'measurementFields' in json_data['event']:
                if  'measurementInterval' in json_data['event']['measurementFields']:
                    measurement_interval = float(json_data['event']['measurementFields']['measurementInterval']);

                if  'additionalFields' in json_data['event']['measurementFields']:
                    if  'SgNB Request Count' in json_data['event']['measurementFields']['additionalFields']:
                        sgnb_req_count = float(json_data['event']['measurementFields']['additionalFields']['SgNB Request Count']);

                    if  'SgNB Accept Count' in json_data['event']['measurementFields']['additionalFields']:
                        sgnb_accpt_count = float(json_data['event']['measurementFields']['additionalFields']['SgNB Accept Count']);

                    if 'Class Id' in json_data['event']['measurementFields']['additionalFields']:
                        class_id = int(json_data['event']['measurementFields']['additionalFields']['Class Id']);
                        

        if measurement_interval > 0:
            print("Class:{0}|Request Rate = {1}|Accept Rate = {2}\n".format(class_id, sgnb_req_count/float(measurement_interval), sgnb_accpt_count/float(measurement_interval)));
                        


#=============================
if __name__ == '__main__':

    conf = {
        '/': {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            'tools.sessions.on': True,
            'tools.response_headers.on':True,
            'tools.response_headers.headers':[('Content-Type','application/json')],
            'tools.staticdir.on': True,
            'tools.staticdir.dir':'/tmp/ves'
        }
     }

    host_name = '127.0.0.1';
    port = 6350;

    cherrypy.config.update({'server.socket_host':host_name, 'server.socket_port':port});
    cherrypy.tree.mount(VES(), '/', config=conf);
    cherrypy.engine.start();
    cherrypy.engine.block();
        
