/////////////////////////////////////////////////////////////
// Simulation of the neuronal network model
//
// This program performs a single simulation run of the model
// described in the manuscript. The network structure is read
// from an input file, and the first module is used as the
// target module for external input.
//
// Two network structures can be used:
//   (1) modular network
//   (2) non-modular network
//
// The corresponding network files can be selected below.
//
// The output contains the temporal activity of all modules
// and the numbers of neurons activated by external and
// synaptic input.
// ////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <math.h>
#include <string>
#include <stdexcept>

using namespace std;

//--------------------------------------------------
// Network and model parameters
//--------------------------------------------------
#define N    2000       // total number of neurons
#define M    8          // number of modules

#define W0E  1          // excitatory synaptic weight
#define W0I  -4         // inhibitory synaptic weight

#define TE   5          // excitatory synaptic activation duration
#define TI   7          // inhibitory synaptic activation duration
#define D    5          // neuronal activation threshold

#define tmax 5000       // total simulation time (time steps)

#define F    0.2        // familiarity: fraction of external input
                         // concentrated in the target module

#define eta  0.001      // total external activation probability

// Heaviside step function
#define H(z)  ( (z>0) ? 1 : 0 )

//--------------------------------------------------
// Synaptic link structure
//
// Only existing network edges are stored. Each link
// contains its target neuron, synaptic weight, and
// remaining activation lifetime.
//--------------------------------------------------
struct Link {
    int target;
    double weight;
    int lifetime;
};

// Sparse adjacency list:
// adj[i] contains all outgoing links from neuron i.
vector<Link> adj[N];

//--------------------------------------------------
// Neuronal state variables
//--------------------------------------------------
int node_state[N] = {0};
int node_input[N] = {0};

// Reason for neuronal activation:
//   0  = inactive
//   1  = active due to external input
//  -1  = active due to synaptic input
int syn_ext[N] = {0};

// Information about the location of module boundaries
// and inhibitory neurons, read from the network input file.
int block_node_number[M+1][4] = {0};

//--------------------------------------------------
int main() {

    // Seed the random-number generator for this simulation run.
    srand(time(NULL));

    int t = 0;

    double r;

    //--------------------------------------------------
    // INPUT: Module structure
    //
    // The first line contains the number of modules and
    // additional information describing the network.
    // Subsequent lines define the neuron boundaries and
    // inhibitory-neuron range of each module.
    //--------------------------------------------------

    // Modular network
    ifstream input_file_blocks("./BlockNodesNumberOfModularNetwork.txt");

    // To use the non-modular network instead, comment the
    // line above and uncomment the following line.
    /*
    ifstream input_file_blocks("./BlockNodesNumberOfNonModularNetwork.txt");
    */

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
    // The network is supplied as an input file rather than
    // generated inside the simulation. This allows the same
    // simulation code to be used with different network
    // realizations.
    //--------------------------------------------------

    // Modular network
    ifstream input_file("./ModularAdjList.txt");

    // To use the non-modular network instead, comment the
    // line above and uncomment the following line.
    /*
    ifstream input_file("./NonModularAdjList.txt");
    */

    int n1,n2,n3,i,j,w;

    // Read network header.
    input_file >> n1 >> n2 >> n3;

    // n2 gives the total number of links in the network.
    int total_link = n2;

    // Read all existing links.
    for (int c=0; c<total_link; c++) {

        input_file >> i >> j >> w;

        Link link;

        link.target = j;
        link.lifetime = 0;

        // The sign of the input weight determines whether
        // the connection is excitatory or inhibitory.
        if (w > 0)
            link.weight = W0E;
        else
            link.weight = W0I;

        adj[i].push_back(link);
    }

    //--------------------------------------------------
    // Properties of the target and non-target modules
    //--------------------------------------------------

    // The first module is used as the target module.
    int N_g = block_node_number[1][1];

    // External activation probability in the target module.
    // F controls the spatial concentration of the input.
    double eta_g = eta * F * M;

    // External activation probability in each non-target module.
    double eta_m = eta * (1-F) * M / (M-1);

    //--------------------------------------------------
    // OUTPUT FILES
    //--------------------------------------------------

    // Fraction of active neurons in each module over time.
    ofstream output("ActivityInTime.txt");

    output << "#Eta = " << eta << "\tF = " << F << endl;
    output << "#time, rho_g, rho_2, ..., rho_8" << endl;

    // Number of neurons activated by external and synaptic
    // input in the target module and in all non-target modules.
    ofstream outputSynExt("NumberOfSynExtInTime.txt");

    outputSynExt << "#Eta = " << eta << "\tF = " << F << endl;
    outputSynExt << "#time,\tN_ext^g,\tN_syn^g,\tN_ext^ms,\tN_syn^ms"
                 << endl;

    //--------------------------------------------------
    // RANDOM INITIAL ACTIVATION AT t = 0
    //--------------------------------------------------

    // Case F = 0:
    // External input is distributed uniformly across all neurons.
    if ( F == 0 ) {

        for (int i = 0; i < N; i++) {

            r = rand()/double(RAND_MAX);

            node_state[i] = H(eta - r);
            syn_ext[i] = node_state[i];
        }
    }

    // Case F != 0:
    // External input is spatially concentrated according to F.
    else {

        // Target module g
        for (int i = 0; i < N_g; i++) {

            r = rand()/double(RAND_MAX);

            node_state[i] = H(eta_g - r);
            syn_ext[i] = node_state[i];
        }

        // Non-target modules
        for (int i = N_g; i < N; i++) {

            r = rand()/double(RAND_MAX);

            node_state[i] = H(eta_m - r);
            syn_ext[i] = node_state[i];
        }
    }

    //--------------------------------------------------
    // OUTPUT AT t = 0
    //--------------------------------------------------

    output << t << "\t";
    outputSynExt << t << "\t";

    double active = 0;

    // Calculate the fraction of active neurons in each module.
    for (int i = 0; i < num1; i++) {

        active = 0;

        for (int j = block_node_number[i][3];
             j < block_node_number[i+1][3];
             j++) {

            active += node_state[j];
        }

        output << active / N << "\t";
    }

    output << endl;

    //--------------------------------------------------
    // Count external and synaptic activation in the
    // target module.
    //--------------------------------------------------

    int N_syn = 0;
    int N_ext = 0;

    for (int i = 0; i < N_g; i++) {

        N_ext += (syn_ext[i] == 1);
        N_syn += (syn_ext[i] == -1);
    }

    outputSynExt << N_ext << "\t" << N_syn << "\t";

    //--------------------------------------------------
    // Count external and synaptic activation in all
    // non-target modules.
    //--------------------------------------------------

    N_syn = 0;
    N_ext = 0;

    for (int i = N_g; i < N; i++) {

        N_ext += (syn_ext[i] == 1);
        N_syn += (syn_ext[i] == -1);
    }

    outputSynExt << N_ext << "\t" << N_syn << endl;

    //--------------------------------------------------
    // NETWORK DYNAMICS
    //--------------------------------------------------

    for (t = 1; t <= tmax; t++) {

        //--------------------------------------------------
        // (1) Update synaptic activation lifetimes
        //
        // An active presynaptic neuron activates an outgoing
        // synapse for TE or TI time steps, depending on the
        // sign of the synaptic weight.
        //--------------------------------------------------

        for (int i = 0; i < N; i++) {

            for (auto &link : adj[i]) {

                int Tmax = (link.weight > 0) ? TE : TI;

                int term1 =
                    H(link.lifetime) * (link.lifetime - 1);

                int term2 =
                    (1 - H(link.lifetime)) *
                    Tmax * node_state[i];

                link.lifetime = (term1 + term2);
            }
        }

        //--------------------------------------------------
        // (2) Compute synaptic input to each neuron
        //--------------------------------------------------

        for (int i = 0; i < N; i++) {

            node_input[i] = 0;
            syn_ext[i] = 0;
        }

        for (int i = 0; i < N; i++) {

            for (auto &link : adj[i]) {

                node_input[link.target] +=
                    H(link.lifetime) * link.weight;
            }
        }

        //--------------------------------------------------
        // (3) Update neuronal states
        //
        // A neuron can become active either through
        // synaptic input or through external input.
        //--------------------------------------------------

        for (int i = 0; i < N; i++) {

            // Activation caused by synaptic input.
            int term1 = H(node_input[i] - D);

            // Activation caused by external input.
            // External activation is applied only when the
            // synaptic input is insufficient to reach threshold.
            int term2 = 0;

            double r = rand()/double(RAND_MAX);

            // Case F = 0:
            // Uniform external activation.
            if ( F == 0 )

                term2 =
                    (1-H(node_input[i]-D)) *
                    H(eta-r);

            // Case F != 0:
            // External activation depends on whether the neuron
            // belongs to the target or a non-target module.
            else {

                // Target module g
                if (i < N_g)

                    term2 =
                        (1-H(node_input[i]-D)) *
                        H(eta_g-r);

                // Non-target modules
                else

                    term2 =
                        (1-H(node_input[i]-D)) *
                        H(eta_m-r);
            }

            // Update neuronal state.
            node_state[i] = term1 + term2;

            // Record the source of activation.
            syn_ext[i] = term2 - term1;
        }

        //--------------------------------------------------
        // (4) OUTPUT AT CURRENT TIME STEP
        //--------------------------------------------------

        output << t << "\t";
        outputSynExt << t << "\t";

        active = 0;

        // Fraction of active neurons in each module.
        for (int i = 0; i < num1; i++) {

            active = 0;

            for (int j = block_node_number[i][3];
                 j < block_node_number[i+1][3];
                 j++) {

                active += node_state[j];
            }

            output << active / N << "\t";
        }

        output << endl;

        //--------------------------------------------------
        // Number of synaptic and external activations
        // in the target module.
        //--------------------------------------------------

        N_syn = 0;
        N_ext = 0;

        for (int i = 0; i < N_g; i++) {

            N_ext += (syn_ext[i] == 1);
            N_syn += (syn_ext[i] == -1);
        }

        outputSynExt << N_ext << "\t" << N_syn << "\t";

        //--------------------------------------------------
        // Number of synaptic and external activations
        // in all non-target modules.
        //--------------------------------------------------

        N_syn = 0;
        N_ext = 0;

        for (int i = N_g; i < N; i++) {

            N_ext += (syn_ext[i] == 1);
            N_syn += (syn_ext[i] == -1);
        }

        outputSynExt << N_ext << "\t" << N_syn << endl;

    } // end of simulation loop

    return 0;
}