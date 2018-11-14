#include "qsmt.h"

using namespace std;

bool qsmt::getLocal()
{

}
qsmt::qsmt(const string& address, const string& name)
{
    _address=address;
    _local=0;
    _model_name=name;
}
qsmt::qsmt()
{
    _address="";
    _local=1;
    _model_name="None";
}

bool qsmt::SMTBatch(string tokens, std::vector< std::allocator >& output_batch_tokens, vector< float >& output_batch_scores)
{
    try {
        string const serverUrl(_address+"/RPC2");
        string const methodName("translate");

        xmlrpc_c::clientSimple myClient;
        xmlrpc_c::value result;
        
        myClient.call(serverUrl, methodName, "text", &result, "This is a test" );

        int const sum = xmlrpc_c::value_int(result);
            // Assume the method returned an integer; throws error if not

        cout << "Result of RPC (sum of 5 and 7): " << sum << endl;

    } catch (exception const& e) {
        cerr << "Client threw error: " << e.what() << endl;
    } catch (...) {
        cerr << "Client threw unexpected error." << endl;
    }
    
}


int
main(int argc, char **) {

    if (argc-1 > 0) {
        cerr << "This program has no arguments" << endl;
        exit(1);
    }

    try {
        string const serverUrl("http://localhost:8080/RPC2");
        string const methodName("sample.add");

        xmlrpc_c::clientSimple myClient;
        xmlrpc_c::value result;
        
        myClient.call(serverUrl, methodName, "ii", &result, 5, 7);

        int const sum = xmlrpc_c::value_int(result);
            // Assume the method returned an integer; throws error if not

        cout << "Result of RPC (sum of 5 and 7): " << sum << endl;

    } catch (exception const& e) {
        cerr << "Client threw error: " << e.what() << endl;
    } catch (...) {
        cerr << "Client threw unexpected error." << endl;
    }

    return 0;
}
