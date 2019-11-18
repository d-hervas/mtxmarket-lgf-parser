#include "../include/instance_parser.h"
/* include block for required libraries */
#include "lemon/smart_graph.h"
#include "lemon/lgf_writer.h"
#include "lemon/maps.h"
#include "lemon/concepts/graph.h"
#include <iostream>
#include <fstream>
#include <map>
#include <string>

/* std namespace functions */
using std::cout;
using std::cerr;
using std::cin;
using std::getline;
using std::vector;
using std::string;
using std::stringstream;
using std::exception;
using std::ifstream;
using std::ios;
using std::map;

/* lemon namespace functions */
using lemon::SmartGraph;
using lemon::IterableIntMap;
using lemon::GraphWriter;
using lemon::INVALID;
using lemon::findEdge;

/* separate the input graph nodes */
std::vector<int> separateNodes (string nodes)
{
	stringstream stream(nodes);
	string buffer;
	vector <int> result;
	while(getline(stream, buffer, '\t'))
	{
		if (result.size() < 2)
		{
			result.push_back(stoi(buffer));
		}
	};
	if (result.size() == 1)
	{
		result.pop_back();
		stream.clear();
		stream.seekg(0);
		while(getline(stream, buffer, ' '))
		{
			if (result.size() < 2)
			{
				result.push_back(stoi(buffer));
			}
		};

	}
	return result;
}

int parseInstance()
{
	cout << "Matrix Market Instance Parser\n";
	cout << "Opening file...\n";
	SmartGraph graph;
	ifstream input_instance("../etc/instance.txt", ios::in);
	if (input_instance.is_open())
	{
		/* we have to create a k/v pair structure to */
		/* save the equivalences between graph IDs and */
		/* instance IDs as defined in the instance text file */
		/* LEMON provides a NodeMap to assign properties to a node, */
	        /* however checking whether the input file ID belongs in that */
		/* map implies iterating over the whole graph to get the node */
		/* object and search in the map itself, which ends up being */ 
		/* worse than just making this myself, I think */ 
		map<int, int> instance_graph_ids;
		string current_line;
		bool prev_comment = false;
		cout << "Opened file, starting loop...\n";
		while (getline (input_instance, current_line))
		{
			if (current_line[0] == '#')
			{
				continue;
				//Skip if the line is a comment
			}
			if (current_line[0] == '%')
			{
				prev_comment = true;
				continue;
				/* in matrix market formats, the first line after the comments */
				/* is a count of the graph's number of edges and nodes. we need */
				/* to skip that first line, else we might get buggy behavior */
			}
			if (prev_comment)
			{
				prev_comment = false;
				continue;
			}
			vector <int> separated_nodes = separateNodes(current_line);
			for_each(separated_nodes.begin(), separated_nodes.end(),[&](int& node)
					{
					if (instance_graph_ids.find(node) == instance_graph_ids.end())
					{
					SmartGraph::Node new_node = graph.addNode();
					int new_node_id = graph.id(new_node);
					instance_graph_ids.insert({ node, new_node_id });
					}
					}
				);
			auto first_node_iterator = instance_graph_ids.find(separated_nodes[0]);
			auto second_node_iterator = instance_graph_ids.find(separated_nodes[1]);
			auto first_node = graph.nodeFromId(first_node_iterator -> second);
			auto second_node = graph.nodeFromId(second_node_iterator -> second);
			if (findEdge(graph, first_node, second_node) == INVALID){
				graph.addEdge( first_node, second_node);
			}
		} 
		IterableIntMap<SmartGraph, SmartGraph::Node> instance_ids(graph);
		for_each(instance_graph_ids.begin(), instance_graph_ids.end(), [&](auto &id_pair)
			{
				instance_ids.set(graph.nodeFromId(id_pair.second), id_pair.first);
			}
		);
		cout << "Writing graph to lgf format...\n";
		graphWriter(graph, "../etc/graph.lgf")
			.nodeMap("instance_ids", instance_ids)
			.run();
		cout << "Done\n";
		return 0;
	} 
	else 
	{
		cerr << "Error opening file. Is the file \"instance.txt\" present?\n";
		return -1;
	}
}
