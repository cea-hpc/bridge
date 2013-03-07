/*
 * bridge.mpmd.wrapper.c - Bridge command wrapper for MPMD launch
 *
 * > For Totalview, a parallel job debugger from
 * TotalView Technologies, LLC <http://www.TotalViewTech.com>
 *      Type "Shift+G" or click "Play" to start the program
 * 
 * Note that you will be requested to validate to stop the different 
 * exec'd binaries sequentially.
 *
 */

extern int bridge_mpmd_wrapper(int argc, char** argv);

int main(int argc, char** argv)
{
	return bridge_mpmd_wrapper(argc,argv);
}
