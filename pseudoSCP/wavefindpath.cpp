extern "C" {
#include "sc_memory_headers.h"
#include "sc_helper.h"
#include "utils.h"
}

#include <stdio.h>
#include <iostream>
#include <glib.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <algorithm>

using namespace std;

sc_memory_context *context;

vector<vector<sc_addr> > chains;

vector<sc_addr> vertices;

vector<sc_addr> getAllVertices(sc_addr);

void getChainByLength(char, int);

void DFSchain(sc_addr, sc_addr, int*, vector<sc_addr>, sc_addr);

int getIndex(sc_addr);

int getWeight(sc_addr, sc_addr);

int toInt(sc_addr);

int main()
{
    sc_memory_params params;

    sc_memory_params_clear(&params);
    params.repo_path = "/home/yegor/ostis/kb.bin";
    params.config_file = "/home/yegor/ostis/config/sc-web.ini";
    params.ext_path = "/home/yegor/ostis/sc-machine/bin/extensions";
    params.clear = SC_FALSE;

    sc_memory_initialize(&params);

    context = sc_memory_context_new(sc_access_lvl_make_max);

    cout<<"G0:"<<endl<<"length of chain = 3"<<endl;
    getChainByLength('0', 3);
    cout<<"----------------"<<endl;

    cout<<"G1:"<<endl<<"length of chain = 5"<<endl;
    getChainByLength('1', 5);
    cout<<"----------------"<<endl;

    cout<<"G2:"<<endl<<"length of chain = 6"<<endl;
    getChainByLength('2', 6);
    cout<<"----------------"<<endl;

    cout<<"G3:"<<endl<<"length of chain = 7"<<endl;
    getChainByLength('3', 7);
    cout<<"----------------"<<endl;

    cout<<"G4:"<<endl<<"length of chain = 3"<<endl;
    getChainByLength('4', 3);
    cout<<"----------------"<<endl;

    sc_memory_context_free(context);

    sc_memory_shutdown(SC_TRUE);

    return 0;
}

vector<sc_addr> getAllVertices(sc_addr graph)
{

    sc_addr rrel_nodes;
    sc_helper_resolve_system_identifier(context, "rrel_nodes", &rrel_nodes);

    vector<sc_addr> vertices;

    sc_iterator5 *sc_vertices = sc_iterator5_f_a_a_a_f_new(context,
                             graph,
                             sc_type_arc_pos_const_perm,
                             0,
                             sc_type_arc_pos_const_perm,
                             rrel_nodes);

    if (SC_TRUE == sc_iterator5_next(sc_vertices)) {
        sc_addr set_vertexes = sc_iterator5_value(sc_vertices, 2);
        sc_iterator3 *it_vertex = sc_iterator3_f_a_a_new(context,
                                  set_vertexes,
                                  sc_type_arc_pos_const_perm,
                                  0);

        while (SC_TRUE == sc_iterator3_next(it_vertex)) {
            sc_addr curr_vertex = sc_iterator3_value(it_vertex, 2);
            vertices.push_back(curr_vertex);
        }

        sc_iterator3_free(it_vertex);
    }

    sc_iterator5_free(sc_vertices);
    return vertices;
}

void getChainByLength(char graph_name, int length)
{

    char gr[3] = "Gx";
    gr[1] = graph_name;

    sc_addr graph, rrel_arcs;

    sc_helper_resolve_system_identifier(context, gr, &graph);
    sc_helper_resolve_system_identifier(context, "rrel_arcs", &rrel_arcs);


    sc_iterator5 *it_arcs = sc_iterator5_f_a_a_a_f_new(context,
                             graph,
                             sc_type_arc_pos_const_perm,
                             0,
                             sc_type_arc_pos_const_perm,
                             rrel_arcs);

    sc_addr arcs;
    if (SC_TRUE == sc_iterator5_next(it_arcs)) {
        arcs = sc_iterator5_value(it_arcs, 2);
    }
    sc_iterator5_free(it_arcs);

    vertices = getAllVertices(graph);
    int V = vertices.size();
    int *color = new int[V];

    for (int i = 0 ; i < V - 1; i++){
        for (int j = i + 1; j < V; j++)
        {
            for (int k = 0; k < V; k++) {
                color[k] = 1;
            }
            vector<sc_addr> simpleChain;
            simpleChain.push_back(vertices[i]);
            DFSchain(vertices[i],vertices[j],color, simpleChain, arcs);
        }
    }

    for (int i = V - 1 ; i > 0; i--){
        for (int j = i - 1 ; j > -1; j--)
        {
            for (int k = 0; k < V; k++) {
                color[k] = 1;
            }
            vector<sc_addr> simpleChain;
            simpleChain.push_back(vertices[i]);
            DFSchain(vertices[i],vertices[j],color, simpleChain, arcs);
        }
    }

    vector<vector<sc_addr> > result;
    for(int i = 0; i < chains.size(); i++){
        int weight = 0;
        for(int j = 0; j < chains[i].size()-1; j++){
            weight += getWeight(chains[i][j], chains[i][j+1]);
        }
        if(weight == length){
            result.push_back(chains[i]);
        }
    }

    if(result.size() == 0){
        cout<<"There are no chains"<<endl;
        return;
    }

    for(int i = 0; i < result.size(); i++){
        for(int j = 0; j < result[i].size()-1; j++){
            printEl(context, result[i][j]);
            cout<<"=>";
            if(j == result[i].size() - 2 ) printEl(context, result[i][j+1]);
        }
        cout<<endl;
    }

    chains.clear();
    vertices.clear();
}

void DFSchain(sc_addr begin, sc_addr end, int *color, vector<sc_addr> simpleChain, sc_addr arcs)
{
    if (!SC_ADDR_IS_EQUAL(begin, end)) {
        color[getIndex(begin)] = 2;
    }

    if(SC_ADDR_IS_EQUAL(begin, end)) {
        chains.push_back(simpleChain);
        return;
    }

    sc_iterator5 *it_vertex = sc_iterator5_f_a_a_a_f_new(context,
                              begin,
                              sc_type_arc_common,
                              0,
                              sc_type_arc_pos_const_perm,
                              arcs);

    if(it_vertex != NULL){

        while (SC_TRUE == sc_iterator5_next(it_vertex)) {
            sc_addr anotherVertex = sc_iterator5_value(it_vertex, 2);

            if (color[getIndex(anotherVertex)] == 1) {
                vector<sc_addr> alternative = simpleChain;
                alternative.push_back(anotherVertex);
                DFSchain(anotherVertex, end, color, alternative, arcs);
                color[getIndex(anotherVertex)] = 1;
            }

        }

    }

}

int getIndex(sc_addr vertex)
{
    int V = vertices.size();

    for(int i = 0; i < V; i++){
        if(SC_ADDR_IS_EQUAL(vertex, vertices[i])){
            return i;
        }
    }

    return -1;
}

int getWeight(sc_addr v1, sc_addr v2)
{
    sc_iterator5 *it = sc_iterator5_f_a_f_a_a_new(context,
                             v1,
                             sc_type_arc_common,
                             v2,
                             sc_type_arc_common,
                             0);

    int weight = -100;

    if(SC_TRUE == sc_iterator5_next(it))
    {
        sc_addr node_weight = sc_iterator5_value(it, 4);
        weight = toInt(node_weight);
    }

    sc_iterator5_free(it);
    return weight;
}

int toInt(sc_addr element)
{
    int number = 0;
    sc_addr idtf;
    sc_type type;
    sc_memory_get_element_type(context, element, &type);

    if ((sc_type_node & type) == sc_type_node)
    {

        if (SC_RESULT_OK == sc_helper_get_system_identifier_link(context, element, &idtf))
        {
            sc_stream *stream;
            sc_uint32 length = 0, read_length = 0;
            sc_char *data;
            sc_memory_get_link_content(context, idtf, &stream);
            sc_stream_get_length(stream, &length);
            data = (sc_char *)calloc(length + 1, sizeof(sc_char));
            sc_stream_read_data(stream, data, length, &read_length);
            data[length] = '\0';
            number = atoi(data);
            sc_stream_free(stream);
            free(data);
        }
    }
    return number;
}
