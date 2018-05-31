#ifndef BDT_H__
#define BDT_H__

#include "ap_fixed.h"

namespace BDT{

constexpr int pow2(int x){
  return x == 0 ? 1 : 2 * pow2(x - 1);
}

constexpr int fn_nodes(int max_depth){
  return pow2(max_depth + 1) - 1;
}

constexpr int fn_leaves(int max_depth){
  return pow2(max_depth);
}

constexpr int fn_classes(int n_classes){
  // Number of trees given number of classes
  return n_classes == 2 ? 1 : n_classes;
}

template<int max_depth, class input_t, class score_t, class threshold_t>
struct Tree {
private:
  static constexpr int n_nodes = fn_nodes(max_depth);
  static constexpr int n_leaves = fn_leaves(max_depth);
public:
	int feature[n_nodes];
	threshold_t threshold[n_nodes];
	score_t value[n_nodes];
	int children_left[n_nodes];
	int children_right[n_nodes];
	int parent[n_nodes];

	score_t decision_function(input_t x) const{
		
		#pragma HLS ARRAY_PARTITION variable=feature complete
		#pragma HLS ARRAY_PARTITION variable=threshold complete
		#pragma HLS ARRAY_PARTITION variable=value complete
		#pragma HLS ARRAY_PARTITION variable=children_left complete
		#pragma HLS ARRAY_PARTITION variable=children_right complete
		#pragma HLS ARRAY_PARTITION variable=parent complete

		#pragma HLS RESOURCE variable=feature core=ROM_2P_LUTRAM
		#pragma HLS RESOURCE variable=threshold core=ROM_2P_LUTRAM
		#pragma HLS RESOURCE variable=value core=ROM_2P_LUTRAM
		#pragma HLS RESOURCE variable=children_left core=ROM_2P_LUTRAM
		#pragma HLS RESOURCE variable=children_right core=ROM_2P_LUTRAM
		#pragma HLS RESOURCE variable=parent core=ROM_2P_LUTRAM

		bool comparison[n_nodes];
		#pragma HLS ARRAY_PARTITION variable=comparison complete
		
		// initialize left/right comparison
		for(int i = 0; i < n_nodes; i++){ comparison[i] = true; }
		// left = true, right = false
		for(int i = 0; i < n_nodes; i++){
			int feature_index = feature[i];
			if (feature_index != -2){
				if (x[feature_index] > threshold[i]) comparison[i] = false;
			}
		}

		int prev_node = 0;
		int curr_node = 0;
		int next_node = 0;
		// std::cout << "max_depth  = " << max_depth << std::endl;
		for (int i = 0; i < (max_depth+1); i++){
			// std::cout << "curr_node = " << curr_node << ", next_node = " << next_node << std::endl;
			curr_node = next_node;
			if (comparison[curr_node]) next_node = children_left[curr_node];
			else next_node = children_right[curr_node];
			if (next_node == -1) return value[curr_node];
		}

		// no decision??
		return -99;

	}

	// score_t decision_function(input_t x) const{
		
	// 	#pragma HLS ARRAY_PARTITION variable=feature complete
	// 	#pragma HLS ARRAY_PARTITION variable=threshold complete
	// 	#pragma HLS ARRAY_PARTITION variable=value complete
	// 	#pragma HLS ARRAY_PARTITION variable=children_left complete
	// 	#pragma HLS ARRAY_PARTITION variable=children_right complete
	// 	#pragma HLS ARRAY_PARTITION variable=parent complete

	// 	#pragma HLS RESOURCE variable=feature core=ROM_nP_LUTRAM
	// 	#pragma HLS RESOURCE variable=threshold core=ROM_nP_LUTRAM
	// 	#pragma HLS RESOURCE variable=value core=ROM_nP_LUTRAM
	// 	#pragma HLS RESOURCE variable=children_left core=ROM_nP_LUTRAM
	// 	#pragma HLS RESOURCE variable=children_right core=ROM_nP_LUTRAM
	// 	#pragma HLS RESOURCE variable=parent core=ROM_nP_LUTRAM

	// 	bool comparison[n_nodes];
	// 	bool activation[n_nodes];
	// 	bool activation_leaf[n_leaves];
	// 	score_t value_leaf[n_leaves];

	// 	#pragma HLS ARRAY_PARTITION variable=comparison
	// 	#pragma HLS ARRAY_PARTITION variable=activation
	// 	#pragma HLS ARRAY_PARTITION variable=activation_leaf
	// 	#pragma HLS ARRAY_PARTITION variable=value_leaf

	// 	// Execute all comparisons
	// 	Compare: for(int i = 0; i < n_nodes; i++){
	// 		#pragma HLS unroll
	// 		// Only non-leaf nodes do comparisons
	// 		//if(x[feature[i]] != -2){ // -2 means is a leaf (at least for sklearn)
	// 		if(feature[i] != -2){ // -2 means is a leaf (at least for sklearn)
	// 			comparison[i] = x[feature[i]] <= threshold[i];
	// 		}else{
	// 			comparison[i] = true;
	// 		}
	// 	}

	// 	// Determine node activity for all nodes
	// 	int iLeaf = 0;
	// 	Activate: for(int i = 0; i < n_nodes; i++){
	// 		#pragma HLS unroll
	// 		// Root node is always active
	// 		if(i == 0){
	// 			activation[i] = true;
	// 		}else{
	// 			// If this node is the left child of its parent
	// 			if(i == children_left[parent[i]]){
	// 				activation[i] = comparison[parent[i]] && activation[parent[i]];
	// 			}else{ // Else it is the right child
	// 				activation[i] = !comparison[parent[i]] && activation[parent[i]];
	// 			}
	// 		}
	// 		// Skim off the leaves
	// 		if(children_left[i] == -1){ // is a leaf
	// 			activation_leaf[iLeaf] = activation[i];
	// 			value_leaf[iLeaf] = value[i];
	// 			iLeaf++;
	// 		}
	// 	}

	// 	score_t y = 0;
	// 	for(int i = 0; i < n_leaves; i++){
	// 		if(activation_leaf[i]){
	// 			// std::cout << "value_leaf[i] = " << value_leaf[i] << std::endl;
	// 			return value_leaf[i];
	// 		}
	// 	}
	// 	return y;
	// }
};

template<int n_trees, int max_depth, int n_classes, class input_t, class score_t, class threshold_t>
struct BDT{

public:
	score_t init_predict[fn_classes(n_classes)];
	Tree<max_depth, input_t, score_t, threshold_t> trees[n_trees][fn_classes(n_classes)];

	void decision_function(input_t x, score_t score[fn_classes(n_classes)]) const{
		for(int j = 0; j < fn_classes(n_classes); j++){
			score[j] = init_predict[j];
		}
		Trees:
		for(int i = 0; i < n_trees; i++){
			Classes:
			for(int j = 0; j < fn_classes(n_classes); j++){
				// std::cout << "j = " << j << ", " << fn_classes(n_classes) << std::endl;
				// printf("specific tree = %0.4f \n", (float) trees[i][j].decision_function(x) );
				score[j] += trees[i][j].decision_function(x);
			}
		}
	}

};

template<int max_depth, class input_t, class score_t, class threshold_t>
constexpr int Tree<max_depth, input_t, score_t, threshold_t>::n_nodes;

template<int max_depth, class input_t, class score_t, class threshold_t>
constexpr int Tree<max_depth, input_t, score_t, threshold_t>::n_leaves;

}
#endif
