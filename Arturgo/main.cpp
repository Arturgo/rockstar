#include "instance.h"
using namespace std;

int main(int nbArgs, char* args[]) {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL); cout.tie(NULL);

    if(nbArgs != 2) {
        cerr << "error: wrong number of arguments" << endl;
        cerr << "usage: ./solver.exe filename" << endl;
        exit(-1);
    }

    string filename(args[1]);
    ifstream fin(filename);

    if(!fin.is_open()) {
        cerr << "error: unknown file " << filename << endl;
        exit(-1);
    }

    cerr << "parsing: " << filename << "... ";

    json data;
    try {
        data = json::parse(fin);
    } catch(json::parse_error& error) {
        cerr << endl << "error: " << error.what() << endl;
        exit(-1);
    }

    Instance inst(data);

    cerr << "done" << endl;
    fin.close();

    cerr << "solving... ";
    Solution solution = inst.solve();
    cerr << "done" << endl;

    double score = solution.score();
    cerr << "score: " << score << endl;
    solution.stat().display();

    json output_data = solution.to_json();

    auto input_path = filesystem::path(filename);
    string output_filename = string(input_path.parent_path()) + "/sol-" + to_string(score) + "-" + string(input_path.filename());

    ofstream fout(output_filename);
    fout << output_data << endl;
    fout.close();

    cerr << "solution written to: " << output_filename << endl; 
    
    return 0;
}
