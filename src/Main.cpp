/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2015 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the top-level of Noxim
 */

#include "ConfigurationManager.h"
#include "NoC.h"
#include "GlobalStats.h"
#include "DataStructs.h"

using namespace std;

// need to be globally visible to allow "-volume" simulation stop
unsigned int drained_volume;

int sc_main(int arg_num, char *arg_vet[])
{
    // TEMP
    drained_volume = 0;

    // Handle command-line arguments
    cout << endl << "\t\tNoxim - the NoC Simulator" << endl;
    cout << "\t\t(C) University of Catania" << endl << endl;

    configure(arg_num, arg_vet);

    //cout << "\n ROUTING = " <<  GlobalParams::routing_algorithm << endl;

    // Signals
    sc_clock clock("clock", 1, SC_NS);
    sc_signal <bool> reset;

    // NoC instance
    NoC *n = new NoC("NoC");

    n->clock(clock);
    n->reset(reset);

    // Trace signals
    sc_trace_file *tf = NULL;
    if (GlobalParams::trace_mode) {
	tf = sc_create_vcd_trace_file(GlobalParams::trace_filename);
	sc_trace(tf, reset, "reset");
	sc_trace(tf, clock, "clock");

	for (int i = 0; i < GlobalParams::mesh_dim_x; i++) {
	    for (int j = 0; j < GlobalParams::mesh_dim_y; j++) {
		char label[30];

		sprintf(label, "req(%02d)(%02d).east", i, j);
		sc_trace(tf, n->req[i][j].east, label);
		sprintf(label, "req(%02d)(%02d).west", i, j);
		sc_trace(tf, n->req[i][j].west, label);
		sprintf(label, "req(%02d)(%02d).south", i, j);
		sc_trace(tf, n->req[i][j].south, label);
		sprintf(label, "req(%02d)(%02d).north", i, j);
		sc_trace(tf, n->req[i][j].north, label);

		sprintf(label, "ack(%02d)(%02d).east", i, j);
		sc_trace(tf, n->ack[i][j].east, label);
		sprintf(label, "ack(%02d)(%02d).west", i, j);
		sc_trace(tf, n->ack[i][j].west, label);
		sprintf(label, "ack(%02d)(%02d).south", i, j);
		sc_trace(tf, n->ack[i][j].south, label);
		sprintf(label, "ack(%02d)(%02d).north", i, j);
		sc_trace(tf, n->ack[i][j].north, label);
	    }
	}
    }
    // Reset the chip and run the simulation
    reset.write(1);
    cout << "Reset..." << endl;
    srand(GlobalParams::rnd_generator_seed);	// time(NULL));
    sc_start(GlobalParams::reset_time, SC_NS);
    reset.write(0);
    cout << " done! Now running for " << GlobalParams::
	simulation_time << " cycles..." << endl;
    sc_start(GlobalParams::simulation_time, SC_NS);

    // Close the simulation
    if (GlobalParams::trace_mode)
	sc_close_vcd_trace_file(tf);
    cout << "Noxim simulation completed." << endl;
    cout << " ( " << sc_time_stamp().to_double() /
	1000 << " cycles executed)" << endl;

    // Show statistics
    GlobalStats gs(n);
    gs.showStats(std::cout, GlobalParams::detailed);

    if ((GlobalParams::max_volume_to_be_drained > 0) &&
	(sc_time_stamp().to_double() / 1000 >=
	 GlobalParams::simulation_time)) {
	cout << endl
         << "WARNING! the number of flits specified with -volume option" << endl
	     << "has not been reached. ( " << drained_volume << " instead of " << GlobalParams::max_volume_to_be_drained << " )" << endl
         << "You might want to try an higher value of simulation cycles" << endl
	     << "using -sim option." << endl;

#ifdef TESTING
	cout << endl
         << " Sum of local drained flits: " << gs.drained_total << endl
	     << endl
         << " Effective drained volume: " << drained_volume;
#endif
    }

    return 0;
}
