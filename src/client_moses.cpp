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
        string const serverUrl("http://localhost:8080/RPC2");
//         string const methodName("translate");

        xmlrpc_c::clientSimple myClient;
        xmlrpc_c::value result;
        xmlrpc_c::paramList myParamList;
//         myParamList.

    string const callXml("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n <methodCall>\r\n  <methodName>translate</methodName>\r\n  <params>\r\n <param><text><s>Ceci est un text</s></text></param>\r\n </params>\r\n  </methodCall>\r\n");

    string methodName;

    xmlrpc_c::xml::parseCall(callXml, &methodName, &myParamList);

    cout << "It is a call of method " << methodName
         << "with " << myParamList.size() << endl;
        
        
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
