// #include <cstdlib>
#include <string>
#include <iostream>

#include <xmlrpc-c/girerr.hpp>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client_simple.hpp>
#include <xmlrpc-c/xml.hpp>

using namespace std;

int main(int argc, char **) {

    if (argc-1 > 0) {
        cerr << "This program has no arguments" << endl;
        exit(1);
    }

    try {
        string const serverUrl("http://localhost:10010/RPC2");
        xmlrpc_c::clientSimple myClient;
        xmlrpc_c::value result;
        xmlrpc_c::paramList myParamList;
        string methodName;
      const string init_value = "<value>";
      const string end_value = "</value>";
      const string init_struct = "<struct>";
      const string end_struct = "</struct>";
      const string init_member = "<member>";
      const string end_member = "</member>";

      string xml_body("<?xml version=\"1.0\" encoding=\"UTF-8\"?><methodCall>  <methodName>translate</methodName>  <params> <param>");
      xml_body.append(init_value);
      xml_body.append(init_struct);
      xml_body.append(init_member);
      xml_body.append("<name>text</name>");
      xml_body.append(init_value);
      xml_body.append("Ceci est un text");
      xml_body.append(end_value);
      xml_body.append(end_member);
      xml_body.append(init_member);
      xml_body.append("<name>report-all-factors</name>");
      xml_body.append(init_value);
      xml_body.append("true");
      xml_body.append(end_value);
      xml_body.append(end_member);
      xml_body.append(init_member);
      xml_body.append("<name>align</name>");
      xml_body.append(init_value);
      xml_body.append("false");
      xml_body.append(end_value);
      xml_body.append(end_member);
      xml_body.append(end_struct);
      xml_body.append(end_value);
      xml_body.append("</param> </params></methodCall>");
// int offset = 0;
// int* offset_ptr = &offset;
// translation_config.fromXml(xml_body,offset_ptr);
// nh.setParam("/translation",translation_config);
  
    xmlrpc_c::xml::parseCall(xml_body, &methodName, &myParamList);
//     methodName="translate";
//     cerr << myParamList[0].type()<< endl;
    cout << xml_body <<endl;
    cout << "It is a call of method " << methodName << " with " << myParamList.size() << endl;
        
        
        myClient.call(serverUrl, methodName, &myParamList, &result);
//         cerr << result <<endl;

//         int const sum = xmlrpc_c::value_int(result);
            // Assume the method returned an integer; throws error if not

//         cout << "Result of RPC (sum of 5 and 7): " << sum << endl;

    } catch (exception const& e) {
        cerr << "Client threw error: " << e.what() << endl;
    } catch (...) {
        cerr << "Client threw unexpected error." << endl;
    }

    return 0;
}
