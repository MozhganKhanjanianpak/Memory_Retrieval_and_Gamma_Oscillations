/////////////////////////////////////////////////////////////
// MODULAR NETWORK SIMULATION
//
// Implementation of the model equations for a discrete-time
// neuronal network with modular connectivity.
//
// The modular network structure is provided as input through
// the node-block information and adjacency-list files. During
// the simulation, one module is randomly selected as the
// target module in each stimulation window. External input is
// distributed between the target and non-target modules
// according to the familiarity parameter F.
//
// The simulation proceeds through alternating stimulation and
// pause intervals. Synaptic activity is updated according to
// the corresponding synaptic lifetimes, followed by neuronal
// state updates and calculation of the activity of each module.
//
// This program performs one network realization. The network
// topology is therefore fixed throughout the simulation, while
// the target module is randomly selected across successive
// stimulation windows.
//
// Output files contain the time-dependent activity of the
// individual modules and the identity of the externally
// stimulated module. These files are used as input to the
// accompanying Python notebook for burst detection,
// stimulus-response classification, and analysis of
// post-stimulus activity persistence.
/////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <math.h>
#include <string>
#include <stdexcept>

using namespace std;

#define N    2000
#define M    8          // Number of modules

#define W0E  1          // Excitatory synaptic weight
#define W0I  -4         // Inhibitory synaptic weight

#define TE   5          // Excitatory synaptic lifetime
#define TI   7          // Inhibitory synaptic lifetime
#define D    5          // Neuronal activation threshold

#define tmax 2000       // Total simulation time

#define F    0.4        // Familiarity with the currently targeted module

#define eta  0.001      // Baseline external activation probability

#define tau_g     15    // Duration of external input to the target module
#define tau_pause 5     // Inter-stimulation interval without external input


#define H(z)  ( (z>0) ? 1 : 0)    // Heaviside function


//--------------------------------------------------
// Link structure
// Only existing edges are stored in the adjacency list.
//--------------------------------------------------
struct Link {
	int target;
	double weight;
	int lifetime;
};

vector<Link> adj[N];               // Adjacency list

int node_state[N] = {0};           // Current state of each neuron
int node_input[N] = {0};           // Total synaptic input to each neuron
int block_node_number[M+1][4] = {0};


//--------------------------------------------------
int main() {

	srand(time(NULL));

	int t = 0;

	double r;


	//--------------------------------------------------
	// INPUT: Modular network structure
	//
	// BlockNodesNumberOfModularNetwork.txt contains:
	//   - number of modules and module properties
	//   - node ranges defining each module
	//--------------------------------------------------
	ifstream input_file_blocks("./BlockNodesNumberOfModularNetwork.txt");

	int num1,num2,num3,num4,bb,nn,Istart,Iend;

	input_file_blocks >> num1 >> num2 >> num3 >> num4;

	block_node_number[0][0] = num1;
	block_node_number[0][1] = num2;
	block_node_number[0][2] = num3;
	block_node_number[0][3] = num4;

	for (int counter=0; counter<num1; counter++) {

		input_file_blocks >> bb >> nn >> Istart >> Iend;

		block_node_number[counter+1][0] = bb;
		block_node_number[counter+1][1] = nn;
		block_node_number[counter+1][2] = Istart;
		block_node_number[counter+1][3] = Iend;
	}


	//--------------------------------------------------
	// INPUT: Network adjacency list
	//
	// ModularAdjList.txt contains the existing network
	// edges. The sign of the input weight determines
	// whether an edge is excitatory or inhibitory.
	//--------------------------------------------------
	ifstream input_file("./ModularAdjList.txt");

	int n1,n2,n3,i,j,w;

	input_file >> n1 >> n2 >> n3;

	int total_link = n2;

	for (int c=0; c<total_link; c++) {

		input_file >> i >> j >> w;

		Link link;

		link.target = j;
		link.lifetime = 0;

		if (w > 0)
			link.weight = W0E;
		else
			link.weight = W0I;

		adj[i].push_back(link);
	}


	//--------------------------------------------------
	// External input probabilities
	//
	// The total external input is redistributed according
	// to the familiarity F:
	//   - target module: eta_g
	//   - other modules: eta_m
	//--------------------------------------------------

	double eta_g = eta * F * M;

	double eta_m = eta * (1-F) * M / (M-1);


	//--------------------------------------------------
	// OUTPUT FILES
	//--------------------------------------------------

	// Activity of each module as a function of time
	ofstream output("ActivityInTime.txt");

	output << "#Eta = " << eta << "\tF = " << F << endl;
	output << "#time, rho_1, rho_2, ..., rho_8" << endl;


	// Identity of the currently targeted module.
	// Zero denotes the inter-stimulation interval.
	ofstream outputTargetModule("TargetModuleInTime.txt");

	outputTargetModule << "#Eta = " << eta << "\tF = " << F << endl;
	outputTargetModule << "0" << endl;


	//--------------------------------------------------
	// INITIAL CONDITION
	//
	// N*eta neurons are randomly activated at t = 0,
	// corresponding to the F = 0 baseline condition.
	//--------------------------------------------------

	int N_0 = N * eta;

	int counter=0;
	int randNode;

	while ( counter < N_0 ) {

		randNode = rand()%N;

		if (node_state[randNode] == 0) {

			node_state[randNode] = 1;
			counter++;
		}
	}


	//--------------------------------------------------
	// OUTPUT AT t = 0
	//--------------------------------------------------

	output << t << "\t";

	double active = 0;         // Number of active neurons

	// Compute activity of each module
	for (int i = 0; i < num1; i++) {

		active = 0;

		for (int j = block_node_number[i][3];
		     j < block_node_number[i+1][3]; j++) {

			active += node_state[j];
		}

		// Normalized by the total network size N
		output << active / N << "\t";
	}

	output << endl;


	//--------------------------------------------------
	// DYNAMICS
	//--------------------------------------------------

	int duration_g;
	int duration_pause;

	t++;

	while ( t<=tmax ) {


		//--------------------------------------------------
		// Select a target module at random
		//--------------------------------------------------

		int target_module = rand()%8+1;

		int start_g = block_node_number[target_module-1][3];
		int end_g   = block_node_number[target_module][3];


		//--------------------------------------------------
		// STIMULATION WINDOW
		//
		// External input is applied to the selected target
		// module for tau_g time steps.
		//--------------------------------------------------

		duration_g=0;

		while ( duration_g<tau_g ) {

			outputTargetModule << target_module << endl;


			//--------------------------------------------------
			// (1) Update synaptic lifetimes
			//
			// A presynaptic spike activates a synapse for
			// TE or TI time steps depending on its type.
			//--------------------------------------------------

			for (int i = 0; i < N; i++) {

				for (auto &link : adj[i]) {

					int Tmax = (link.weight > 0) ? TE : TI;

					int term1 =
						H(link.lifetime) * (link.lifetime - 1);

					int term2 =
						(1 - H(link.lifetime)) *
						Tmax * node_state[i];

					link.lifetime = term1 + term2;
				}
			}


			//--------------------------------------------------
			// (2) Compute total synaptic input
			//--------------------------------------------------

			for (int i = 0; i < N; i++)
				node_input[i] = 0;

			for (int i = 0; i < N; i++) {

				for (auto &link : adj[i]) {

					node_input[link.target] +=
						H(link.lifetime) * link.weight;
				}
			}


			//--------------------------------------------------
			// (3) Update neuronal states
			//
			// A neuron can become active either through:
			//   (i) synaptic input exceeding threshold D, or
			//  (ii) stochastic external activation.
			//--------------------------------------------------

			for (int i = 0; i < N; i++) {

				// Activation due to synaptic input
				int term1 = H(node_input[i] - D);

				// Stochastic external activation
				int term2 = 0;

				double r = rand()/double(RAND_MAX);


				// F = 0: homogeneous external input
				if ( F == 0 )

					term2 =
						(1- H(node_input[i] - D)) *
						H(eta - r);


				// F != 0: input depends on module identity
				else {

					// Target module
					if (i>=start_g && i<end_g)

						term2 =
							(1- H(node_input[i] - D)) *
							H(eta_g - r);

					// Non-target modules
					else

						term2 =
							(1- H(node_input[i] - D)) *
							H(eta_m - r);
				}


				// Neuron is active if either input mechanism
				// crosses its corresponding threshold.
				node_state[i] = term1 + term2;
			}


			//--------------------------------------------------
			// (4) Record activity
			//--------------------------------------------------

			output << t << "\t";

			active = 0;

			for (int i = 0; i < num1; i++) {

				active = 0;

				for (int j = block_node_number[i][3];
				     j < block_node_number[i+1][3]; j++) {

					active += node_state[j];
				}

				output << active / N << "\t";
			}

			output << endl;


			t++;
			duration_g++;

		} // End of stimulation window


		//--------------------------------------------------
		// POST-STIMULATION INTERVAL
		//
		// No external input is applied for tau_pause
		// time steps. Activity is driven only by synaptic
		// interactions.
		//--------------------------------------------------

		duration_pause=0;

		while ( duration_pause<tau_pause ) {

			target_module=0;

			// Zero indicates that no module is being stimulated
			outputTargetModule << target_module << endl;


			//--------------------------------------------------
			// (1) Update synaptic lifetimes
			//--------------------------------------------------

			for (int i = 0; i < N; i++) {

				for (auto &link : adj[i]) {

					int Tmax = (link.weight > 0) ? TE : TI;

					int term1 =
						H(link.lifetime) * (link.lifetime - 1);

					int term2 =
						(1 - H(link.lifetime)) *
						Tmax * node_state[i];

					link.lifetime = term1 + term2;
				}
			}


			//--------------------------------------------------
			// (2) Compute total synaptic input
			//--------------------------------------------------

			for (int i = 0; i < N; i++)
				node_input[i] = 0;

			for (int i = 0; i < N; i++) {

				for (auto &link : adj[i]) {

					node_input[link.target] +=
						H(link.lifetime) * link.weight;
				}
			}


			//--------------------------------------------------
			// (3) Update neuronal states
			//
			// During the pause interval there is no external
			// input, so activation occurs only through synaptic
			// input.
			//--------------------------------------------------

			for (int i = 0; i < N; i++)

				node_state[i] =
					H(node_input[i] - D);


			//--------------------------------------------------
			// (4) Record activity
			//--------------------------------------------------

			output << t << "\t";

			active = 0;

			for (int i = 0; i < num1; i++) {

				active = 0;

				for (int j = block_node_number[i][3];
				     j < block_node_number[i+1][3]; j++) {

					active += node_state[j];
				}

				output << active / N << "\t";
			}

			output << endl;


			t++;
			duration_pause++;

		} // End of post-stimulation interval

	} // End of dynamical loop


	return 0;
}