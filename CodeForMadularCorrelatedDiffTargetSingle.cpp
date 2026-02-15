////////////////////////////////////////////////
// Implementation code for Madular netwok with
// correlated input
////////////////////////////////////////////////
using namespace std;

#include <string.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <fstream>
#include <cstdlib>  // defines rand
#include <ctime>    // defines time() function

#define linkState(x)  ( (x==0) ? 0 : 1)  // inline function

#define N    2000   // Number of nodes
#define B    8      // number of modules (M)
#define E    0.8    // percentage of Excitatory neurons
#define TE   5      // activation time of Exc. links
#define TI   7      // activation time of Inh. links

#define F   0.4    // Familiarity parameter between 0 and 1
#define tau_g   30      // duration of injecting current based on L to the target module
#define tau_pause  5 // duration of no injection (inter pauses between choosing the target madule)

#define eta  0.001   // eta value

#define D    6     // thereshold value for firing
#define tmax 800    // maximum time

int A[N][N] = {0};               // Adjacency Matrix
int node_state[N] = {0};          // state of neurons, including: 0=inactive , 1=active
int node_input[N] = {0};          // neuron input
int block_node_number[B+1][4] = {0};   // array for saving the number of nodes in each block
int Link_counter[N][N]         = {0}; // including 0, 1, 2, ...,TE for 0<row<n_E and 0<col<N
int Link_counter_updated[N][N] = {0}; // and 0, 1, 2, ...,TI for n_E<row<N and 0<col<N

/********************************************/
// Main
/********************************************/
int main() {
	unsigned seed = time(NULL);
	srand(seed);
	double r;       // double random number between (0,1)
	int t=0;        // time
	int counter;

	//-----------------------------------------------
	// INPUT; the number of neurons in each block and the label of Inhibitory ones
	ifstream input_file_blocks("./BlockNodesNumber.txt");
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

	//-----------------------------------------------
	// INPUT; the adjacency matrix A
	ifstream input_file("./MadularAdjList.txt");
	int n1,n2,n3,i,j,k;
	input_file >> n1 >> n2 >> n3 ;
	for (int counter=0; counter<n2; counter++) {
		input_file >> i >> j >> k;
		A[i][j] = k;
	}

	//------------------------------------------------
	int N_0=N*eta; // initial number of active neurons
	double eta_g = eta*F*B; // linear eta for module g in case l!=0
	double eta_m = eta*(1-F)*B/(B-1); // linear eta for other modules in case l!=0

	// output files
	string name = "./ActiveDens.txt";
	ofstream outputExcInh(name); //output for active E+I neurons
	outputExcInh << "#time,	Rho for module 1 to module " << B << " when F = " << F << endl;

	// output file of target module according to parameters
	string Lname="./TargetModulesInTime.txt";
	ofstream outputTargetModule(Lname); //output for active E+I neurons
	outputTargetModule << "0" << endl;

	//------------------------------------------------
	// becomming active (RANDOMLY) at t=0
	// always with assumption of F=0
	counter=0;
	int randNode;
	while ( counter < N_0 ) {
		randNode = rand()%N;  // a random neuron
		if (node_state[randNode] == 0) {
			node_state[randNode]=1;
			counter++;
		}
	}

	//------------------------------------------------
	//output at t=0
	//Excitatoty and Inhibitory neurons
	outputExcInh << t << "\t";
	double act_density=0; // fraction of active neurons in a module
	for (int i=0 ; i<num1 ; i++) {
		act_density=0;
		for (int j=block_node_number[i][3] ; j<block_node_number[i+1][3] ; j++) {
			act_density += node_state[j];
		}
		outputExcInh << act_density/N << "\t";
	}
	outputExcInh << endl;

	//------------------------------------------------
	// Starting Dynamics
	int duration_g;
	int duration_pause;
	t++;
	while ( t<=tmax ) {
		duration_g=0;

		// setting target module randomly
		int target_module = rand()%8+1;  // a random number between 1 and 8
		int start_g = block_node_number[target_module-1][3]; // first neuron of target module
		int end_g = block_node_number[target_module][3]; // last neuron of target module

		// external current injection duration for target module
		while ( duration_g<tau_g ) {

			outputTargetModule << target_module << endl;

			// updating link-counters
			for (int i=0 ; i<N ; i++) {
				for (int j=0 ; j<N ; j++) {

					// for Excitatory links --> A[i][j] == 0 or +1
					if ( A[i][j]==1 ) {

						// links that remain inactive
						if ( (node_state[i]==0) && (Link_counter[i][j]==0)  )
							Link_counter_updated[i][j] = 0;

						// links that become active as a result of activation of thier pre-synaptic neuron
						else if ( (node_state[i]==1) && (Link_counter[i][j]==0)  )
							Link_counter_updated[i][j] = 1;

						// active links whose time counter is less than TE
						else if ( (Link_counter[i][j] >= 1) && (Link_counter[i][j] < TE) )
							Link_counter_updated[i][j] = Link_counter[i][j] + 1;

						// active links whose time counter is TE --> become inactive
						else if ( Link_counter[i][j] == TE )
							Link_counter_updated[i][j] = 0;
					}

					// for Inhibitory links --> A[i][j] == 0 or -1
					if ( A[i][j]==-1 ) {

						// links that remain inactive
						if ( (node_state[i]==0) && (Link_counter[i][j]==0)  )
							Link_counter_updated[i][j] = 0;

						// links that become active as a result of activation of thier pre-synaptic neuron
						else if ( (node_state[i]==1) && (Link_counter[i][j]==0)  )
							Link_counter_updated[i][j] = 1;

						// active links whose time counter is less than TI
						else if ( (Link_counter[i][j] >= 1) && (Link_counter[i][j] < TI) )
							Link_counter_updated[i][j] = Link_counter[i][j] + 1;

						// active links whose time counter is TI --> become inactive
						else if ( Link_counter[i][j] == TI )
							Link_counter_updated[i][j] = 0;
					}
				}
			}

			// updating node states
			// First: node inputs
			for (int i=0 ; i<N ; i++) {
				node_input[i] = 0;
				int positive_input=0;
				int negative_input=0;
				for (int j=0 ; j<N ; j++) {
					if (A[j][i]==1)
						positive_input += linkState(Link_counter_updated[j][i]);
					if (A[j][i]==-1)
						negative_input += linkState(Link_counter_updated[j][i]);
				}
				// total input
				node_input[i] = positive_input - 4*negative_input;
			}
			// Then: node-state
			for (int i=0 ; i<N ; i++) {
				node_state[i]=0;
				if ( node_input[i] >= D )
					node_state[i]=1;
			}

			// external input on nodes at t
			// case F=0
			if (F==0) {
				// all modules
				for (int i=0 ; i<N ; i++) {
					r = rand()/double(RAND_MAX);
					if (r<eta) node_state[i]=1;
				}
			}
			// case F!=0
			else {
				for (int i=0 ; i<N ; i++) {
					r = rand()/double(RAND_MAX);
					// target module g
					if (i>=start_g && i<end_g)
						if (r<eta_g)
							node_state[i]=1;
					// modules m (non-target)
					else
						if (r<eta_m)
							node_state[i]=1;
				}
			}

			//output at t
			//Excitatoty and Inhibitory neurons
			outputExcInh << t << "\t";
			act_density=0; // fraction of active neurons in a module
			for (int i=0 ; i<num1 ; i++) {
				act_density=0;
				for (int j=block_node_number[i][3] ; j<block_node_number[i+1][3] ; j++) {
					act_density += node_state[j];
				}
				outputExcInh << act_density/N << "\t";
			}
			outputExcInh << endl;

			//updating for the next time step
			t++;
			//if (t>tmax) break;
			for (int i=0 ; i<N ; i++) {
				for (int j=0; j<N ; j++) {
					Link_counter[i][j] = Link_counter_updated[i][j];
				}
			}
			duration_g++;
		} // end of while for tau_g duration

		// pause times (no injection)
		duration_pause=0;
		while ( duration_pause<tau_pause) {

			target_module=0;
			outputTargetModule << target_module << endl;

			for (int i=0 ; i<N ; i++) {
				for (int j=0 ; j<N ; j++) {

					// for Excitatory links --> A[i][j] == 0 or +1
					if ( A[i][j]==1 ) {

						// links that remain inactive
						if ( (node_state[i]==0) && (Link_counter[i][j]==0)  )
							Link_counter_updated[i][j] = 0;

						// links that become active as a result of activation of thier pre-synaptic neuron
						else if ( (node_state[i]==1) && (Link_counter[i][j]==0)  )
							Link_counter_updated[i][j] = 1;

						// active links whose time counter is less than TE
						else if ( (Link_counter[i][j] >= 1) && (Link_counter[i][j] < TE) )
							Link_counter_updated[i][j] = Link_counter[i][j] + 1;

						// active links whose time counter is TE --> become inactive
						else if ( Link_counter[i][j] == TE )
							Link_counter_updated[i][j] = 0;
					}

					// for Inhibitory links --> A[i][j] == 0 or -1
					if ( A[i][j]==-1 ) {

						// links that remain inactive
						if ( (node_state[i]==0) && (Link_counter[i][j]==0)  )
							Link_counter_updated[i][j] = 0;

						// links that become active as a result of activation of thier pre-synaptic neuron
						else if ( (node_state[i]==1) && (Link_counter[i][j]==0)  )
							Link_counter_updated[i][j] = 1;

						// active links whose time counter is less than TI
						else if ( (Link_counter[i][j] >= 1) && (Link_counter[i][j] < TI) )
							Link_counter_updated[i][j] = Link_counter[i][j] + 1;

						// active links whose time counter is TI --> become inactive
						else if ( Link_counter[i][j] == TI )
							Link_counter_updated[i][j] = 0;
					}
				}
			}

			// updating node states
			// First: node inputs
			for (int i=0 ; i<N ; i++) {
				node_input[i] = 0;
				int positive_input=0;
				int negative_input=0;
				for (int j=0 ; j<N ; j++) {
					if (A[j][i]==1)
						positive_input += linkState(Link_counter_updated[j][i]);
					if (A[j][i]==-1)
						negative_input += linkState(Link_counter_updated[j][i]);
				}
				// total input
				node_input[i] = positive_input - 4*negative_input;
			}
			// Then: node-state
			for (int i=0 ; i<N ; i++) {
				node_state[i]=0;
				if ( node_input[i] >= D )
					node_state[i]=1;
			}

			//output at t
			//Excitatoty and Inhibitory neurons
			outputExcInh << t << "\t";
			act_density=0; // fraction of active neurons in a module
			for (int i=0 ; i<num1 ; i++) {
				act_density=0;
				for (int j=block_node_number[i][3] ; j<block_node_number[i+1][3] ; j++) {
					act_density += node_state[j];
				}
				outputExcInh << act_density/N << "\t";
			}
			outputExcInh << endl;

			//updating for the next time step
			t++;
			//if (t>tmax) break;
			for (int i=0 ; i<N ; i++) {
				for (int j=0; j<N ; j++) {
					Link_counter[i][j] = Link_counter_updated[i][j];
				}
			}
			duration_pause++;
		} // end of while for tau_pause duration
	} // end of while
	return 0;
}
