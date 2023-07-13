/*****************************************************************************\
 *  lib/bridge/bridge_common.h - 
 *****************************************************************************
 *  Copyright  CEA/DAM/DIF (2012)
 *
 *  This file is part of Bridge, an abstraction layer to ease batch system and
 *  resource manager usage in heterogeneous HPC environments.
 *
 *  Bridge is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Bridge is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Bridge.  If not, see <http://www.gnu.org/licenses/>
\*****************************************************************************/

#ifndef _BRIDGE_COMMON_H
#define _BRIDGE_COMMON_H


/* define a macro that make an advanced free on pointer */
#define xfree(a) \
if(a!=NULL){ \
  free(a); \
  a=NULL; \
} 

/*! \addtogroup BRIDGE_RANGE
 *  @{
 */
/*!
 * \ingroup BRIDGE_RANGE
 * \brief structure that represent a range of long int value
 */
typedef struct bridge_range {
  long int from; /*!<
		  * start point of the range (included in the range)
		  */
  long int to; /*!<
		* end point of the range (included in the range)
		*/
} bridge_range_t ;
/*!
 * \ingroup BRIDGE_RANGE
 * \brief set bridge range values
 *
 * \param r1 input range
 * \param v1 from value
 * \param v2 to value
 *
 * \retval  0 should not return an other value
*/
int
bridge_range_set(bridge_range_t* r1, long int v1, long int v2);
/*!
 * \ingroup BRIDGE_RANGE
 * \brief Indicate if the range is a valid one. That is to say if from value
 * is lower that to value
 *
 * \param r1 input range
 *
 * \retval  1 if the range is valid
 * \retval  0 if the range is not valid
*/
int
bridge_range_check(bridge_range_t* r1);
/*!
 * \ingroup BRIDGE_RANGE
 * \brief Indicate if the first range equals, is placed before
 * or is placed after the second one
 *
 * \param a1 one of the two input ranges
 * \param a2 one of the two input ranges
 *
 * \retval  1 if the second one end before the start of the first one
 * \retval  0 if the two ranges are equals
 * \retval -1 if the second one start after the end of the first one
*/
int
bridge_range_compare(bridge_range_t* r1,bridge_range_t* r2);
/*!
 * \ingroup BRIDGE_RANGE
 * \brief Indicate if it exists an non empty intersection
 * between the two input ranges
 *
 * \param r1 one of the two input ranges
 * \param r2 one of the two input ranges
 *
 * \retval  1 an intersection exists
 * \retval  0 otherwise
*/
int
bridge_range_intersects(bridge_range_t* r1,bridge_range_t* r2);
/*!
 * \ingroup BRIDGE_RANGE
 * \brief Gives the range that is common to the two input ranges
 *
 * \param r1 one of the two input ranges
 * \param r2 one of the two input ranges
 * \param r3 output range
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_range_intersection(bridge_range_t* r1,bridge_range_t* r2,bridge_range_t* r3);
/*!
 * \ingroup BRIDGE_RANGE
 * \brief Indicate if the two input ranges are contiguous
 *
 * \param r1 one of the two input ranges
 * \param r2 one of the two input ranges
 *
 * \retval  0 if not continuous
 * \retval  1 if continuous and r1 is before r2
 * \retval  2 if continuous and r2 is before r1
*/
int
bridge_range_contiguous(bridge_range_t* r1,bridge_range_t* r2);
/*!
 * \ingroup BRIDGE_RANGE
 * \brief Indicate if one of the two input range is included in the other
 * one
 *
 * \param r1 one of the two input ranges
 * \param r2 one of the two input ranges
 *
 * \retval  0 if no inclusion detected
 * \retval  1 if r2 is included in r1 (r1 is the bigger one)
 * \retval  2 if r1 is included in r2 (r2 is the bigger one)
*/
int
bridge_range_includes(bridge_range_t* r1,bridge_range_t* r2);
/*!
 * \ingroup BRIDGE_RANGE
 * \brief Gives a bridge_range that represent the union of the two bridge_ranges
 * given in input. The two ranges must intersect or be continuous otherwise
 * operation will failed
 *
 * \param r1 one of the two input ranges
 * \param r2 one of the two input ranges
 * \param r3 output range
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_range_union(bridge_range_t* r1,bridge_range_t* r2,bridge_range_t* rout);
/*!
 * @}
*/


/*! \addtogroup BRIDGE_RANGES_ARRAY
 *  @{
 */
/*!
 * \ingroup BRIDGE_RANGES_ARRAY
 * \brief structure that represent a range of long int value
 */
typedef struct bridge_rangelist {

  long int ranges_nb; /*!<
		       * quantity of ranges currently stored in array
		       */
  bridge_range_t* array; /*!<
		       * array of ranges
		       */

  size_t pre_allocated_ranges; /*!<
				* quantity of pre-allocated ranges of the array
				*/

} bridge_rangelist_t;
/*!
 * \ingroup BATCH_MANAGER
 * \brief Initialize a bridge ranges array structure
 *
 * \param array pointer on a bridge ranges array structure to initialize
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int bridge_rangelist_init(bridge_rangelist_t* array);
/*!
 * \ingroup BATCH_MANAGER
 * \brief Initialize a bridge ranges array structure by duplicating an other one
 *
 * \param array pointer on a bridge ranges array structure to initialize
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int bridge_rangelist_init_by_copy(bridge_rangelist_t* array,bridge_rangelist_t* a2c);
/*!
 * \ingroup BATCH_MANAGER
 * \brief Free a bridge ranges array structure contents
 *
 * \param array pointer on a bridge ranges array structure to finalize
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int bridge_rangelist_free_contents(bridge_rangelist_t* array);
/*!
 * \ingroup BATCH_MANAGER
 * \brief Increment a bridge ranges array storage zone
 *
 * \param array pointer on a bridge ranges array structure to increment
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int bridge_rangelist_incremente_size(bridge_rangelist_t* array);
/*!
 * \ingroup BATCH_MANAGER
 * \brief Add a range to a bridge ranges array
 * The range will be merge with already existing ranges if required
 * and resulting ranges will be sorted
 *
 * \param array pointer on a bridge ranges array structure to use for add-on
 * \param r range that will be add to the array
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int bridge_rangelist_add_range(bridge_rangelist_t* array,bridge_range_t* r);
/*!
 * \ingroup BATCH_MANAGER
 * \brief Add a rangelist to a bridge ranges array
 * The range will be merge with already existing ranges if required
 * and resulting ranges will be sorted
 *
 * \param array pointer on a bridge ranges array structure to use for add-on
 * \param rlin rangelist that will be added to the array
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int bridge_rangelist_add_rangelist(bridge_rangelist_t* array,
				   bridge_rangelist_t* rlin);
/*!
 * \ingroup BATCH_MANAGER
 * \brief Remove a rangelist from a bridge ranges array
 *
 * \param array pointer on a bridge ranges array structure to use
 * \param rlin rangelist that will be removed to the array
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int bridge_rangelist_remove_rangelist(bridge_rangelist_t* array,
				      bridge_rangelist_t* rlin);
/*!
 * \ingroup BATCH_MANAGER
 * \brief Add a list of values to a bridge ranges array
 * The range will be merge with already existing ranges if required
 * and resulting ranges will be sorted
 *
 * \param array pointer on a bridge ranges array structure to use for add-on
 * \param list values list that must be added (pattern [*,*-*...])
 *
 * \retval  n padding if operation successfully done
 * \retval -1 operation failed
*/
int bridge_rangelist_add_list(bridge_rangelist_t* array,char* list);
/*!
 * \ingroup BATCH_MANAGER
 * \brief Sort a bridge ranges array
 *
 * \param array pointer on a bridge ranges array structure to sort
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
 */
int bridge_rangelist_sort(bridge_rangelist_t* array);
/*!
 * @}
*/



/*! \addtogroup BRIDGE_IDS_LIST
 *  @{
 */
/*!
 * \ingroup BRIDGE_IDS_LIST
 * \brief structure that represent a range of long int value
 */
typedef struct bridge_idlist {

  long int id_nb;

  bridge_rangelist_t rangelist; /*!<
				    * ranges array of this list
				    */
} bridge_idlist_t;
/*!
 * \ingroup BRIDGE_IDLIST
 * \brief Initialize a bridge ids list structure
 *
 * \param idlist pointer on a bridge ids list structure to initialize
 * \param lists array of strings containing ids to add to this list
 * \param lists_nb quanity of string in the array
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_idlist_init(bridge_idlist_t* idlist,char** lists, int lists_nb);
/*!
 * \ingroup BRIDGE_IDLIST
 * \brief Free a bridge ids list structure
 *
 * \param idlist pointer on a bridge ids list structure to finalize
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_idlist_free_contents(bridge_idlist_t* idlist);
/*!
 * \ingroup BRIDGE_IDLIST
 * \brief Add a ids list to a bridge ids list structure
 *
 * \param idlist pointer on a bridge ids list structure
 * \param list ids list to add to this bridge ids list
 *
 * \retval  n padding length if operation successfully done
 * \retval -1 operation failed
*/
int
bridge_idlist_add_ids(bridge_idlist_t* idlist,char* list);
/*!
 * \ingroup BRIDGE_IDLIST
 * \brief Get ids quantity
 *
 * \param idlist pointer on a bridge ids list structure
 *
 * \retval quantity of ids in this bridge ids list
*/
long int
bridge_idlist_ids_quantity(bridge_idlist_t* idlist);
/*!
 * \ingroup BRIDGE_IDLIST
 * \brief Get extended ids string
 *
 * \param idlist pointer on a bridge ids list structure
 * \param p_string pointer on a string that will be allocated and filled with ids names
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_idlist_get_extended_string(bridge_idlist_t* idlist,char** p_string);
/*!
 * \ingroup BRIDGE_IDLIST
 * \brief Get compacted ids string
 *
 * \param idlist pointer on a bridge ids list structure
 * \param p_string pointer on a string that will be allocated and filled with compacted ids list
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_idlist_get_compacted_string(bridge_idlist_t* idlist,char** p_string);
/*!
 * @}
*/

/*! \addtogroup BRIDGE_NODEPATTERN
 *  @{
 */
/*!
 * \brief structure that represent a nodename pattern
 *
 * can be use for basic node or enumartion node (prefixXXXsuffix pattern)
 */
typedef struct bridge_nodepattern {
  int padding; /*!< padding length */
  char* prefix; /*!< nodename prefix */
  char* suffix; /*!< nodename suffix */
  int basic; /*!< basic node flag 0=no 1=yes, basic node is not part of an node enumeration */
} bridge_nodepattern_t;
/*!
 * \brief Initialize a bridge node pattern structure
 *
 * by default, padding is set to 0, prefix and suffix to NULL
 * and the node pattern is basic
 *
 * \param np pointer on a bridge node pattern structure to initialize
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_nodepattern_init(bridge_nodepattern_t* np);
/*!
 * \brief Initialize a bridge node pattern structure by dumping an other one
 *
 * by default, padding is set to 0, prefix and suffix to NULL
 * and the node pattern is basic
 *
 * \param np pointer on a bridge node pattern structure to initialize
 * \param npin pointer on a bridge node pattern to copy
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_nodepattern_init_by_copy(bridge_nodepattern_t* np,bridge_nodepattern_t* npin);
/*!
 * \brief Clean a bridge node pattern structure
 *
 * \param np pointer on a bridge node pattern structure to free
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_nodepattern_free_contents(bridge_nodepattern_t* np);
/*!
 * \brief Set bridge node pattern padding
 *
 * \param np pointer on a bridge node pattern structure to free
 * \param padding padding value of the pattern
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_nodepattern_set_padding(bridge_nodepattern_t* np,int padding);
/*!
 * \brief Set bridge node pattern prefix
 *
 * \param np pointer on a bridge node pattern structure
 * \param prefix node pattern prefix
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_nodepattern_set_prefix(bridge_nodepattern_t* np,char* prefix);
/*!
 * \brief Set bridge node pattern prefix
 *
 * \param np pointer on a bridge node pattern structure
 * \param prefix node pattern prefix
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_nodepattern_set_suffix(bridge_nodepattern_t* np,char* suffix);
/*!
 * \brief Set bridge node pattern basic flag
 *
 * \param np pointer on a bridge node pattern structure
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_nodepattern_set_basic(bridge_nodepattern_t* np);
/*!
 * \brief Unset bridge node pattern basic flag
 *
 * \param np pointer on a bridge node pattern structure
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_nodepattern_unset_basic(bridge_nodepattern_t* np);
/*!
 * \brief Test if two bridge node patterns are identical
 *
 * \param np1 pointer on the first bridge node pattern structure
 * \param np2 pointer on the first bridge node pattern structure
 *
 * \retval  1 if the two pattern are identical
 * \retval  0 if they are not identical
*/
int
bridge_nodepattern_equals(bridge_nodepattern_t* np1,bridge_nodepattern_t* np2);
/*!
 * @}
*/


/*! \addtogroup BRIDGE_NODELIST
 *  @{
 */
/*!
 * \ingroup BRIDGE_NODELIST
 * \brief structure that represent a range of long int value
 */
typedef struct bridge_nodelist
{
  bridge_nodepattern_t pattern; /*!<
				 * ranges array of this list
				 */
  bridge_rangelist_t rangelist; /*!<
				 * ranges array of this list
				 */
  struct bridge_nodelist* next; /*!<
				 * next node list or NULL 
				 * if no more node list aggregated
				 */
} bridge_nodelist_t;
/*!
 * \ingroup BRIDGE_NODELIST
 * \brief Initialize a bridge nodes list structure
 *
 * \param nodelist pointer on a bridge nodes list structure to initialize
 * \param lists array of strings containing nodes to add to this list
 * \param lists_nb quanity of string in the array
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_nodelist_init(bridge_nodelist_t* nodelist,char** lists, int lists_nb);
/*!
 * \ingroup BRIDGE_NODELIST
 * \brief Free a bridge nodes list structure
 *
 * \param nodelist pointer on a bridge nodes list structure to finalize
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_nodelist_free_contents(bridge_nodelist_t* nodelist);
/*!
 * \ingroup BRIDGE_NODELIST
 * \brief Add a nodes list to a bridge nodes list structure
 *
 * \param nodelist pointer on a bridge nodes list structure
 * \param list nodes list to add to this bridge nodes list
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_nodelist_add_nodes(bridge_nodelist_t* nodelist,char* list);
/*!
 * \ingroup BRIDGE_NODELIST
 * \brief Check a nodes list versus another one according to required operation
 * operation can be inclusion or intersection
 *
 * \param nodelist pointer on the first bridge nodes list structure
 * \param nodelist pointer on the second bridge nodes list structure
 * \param operation on of VERSUS_OPERATION_INCLUDE or VERSUS_OPERATION_INTERSECT
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
#define VERSUS_OPERATION_INCLUDE         1
#define VERSUS_OPERATION_INTERSECT       2
int
bridge_nodelist_versus_second_list(bridge_nodelist_t* nodelist,bridge_nodelist_t* second_list,int operation);
/*!
 * \ingroup BRIDGE_NODELIST
 * \brief Get nodes quantity
 *
 * \param nodelist pointer on a bridge nodes list structure
 *
 * \retval quantity of nodes in this bridge nodes list
*/
long int
bridge_nodelist_nodes_quantity(bridge_nodelist_t* nodelist);
/*!
 * \ingroup BRIDGE_NODELIST
 * \brief Get extended nodes string
 *
 * \param nodelist pointer on a bridge nodes list structure
 * \param p_string pointer on a string that will be allocated and filled with nodes names
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_nodelist_get_extended_string(bridge_nodelist_t* nodelist,char** p_string);
/*!
 * \ingroup BRIDGE_NODELIST
 * \brief Get compacted nodes string
 *
 * \param nodelist pointer on a bridge nodes list structure
 * \param p_string pointer on a string that will be allocated and filled with compacted nodes list
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_nodelist_get_compacted_string(bridge_nodelist_t* nodelist,char** p_string);

/*!
 * \ingroup BRIDGE_NODELIST
 * \brief Get an extended nodes list based on a compacted one
 *
 * \param src_list compacted (or not) nodes list
 * \param p_dst_list extended nodes list
 *
 * \retval  n nodes quantity if operation successfully done
 * \retval -1 operation failed
*/
int
bridge_common_condensed2extended_nodelist(char* src_list,char** p_dst_list);

/*!
 * \ingroup BRIDGE_NODELIST
 * \brief Get an compacted nodes list based on a extended one
 *
 * \param src_list extended (or not) nodes list
 * \param p_dst_list compacted nodes list
 *
 * \retval  n nodes quantity if operation successfully done
 * \retval -1 operation failed
*/
int
bridge_common_extended2condensed_nodelist(char* src_list,char** p_dst_list);
/*!
 * \ingroup BRIDGE_COMMON
 * \brief Check if two nodes list intersect
 *
 * \param first_list first nodes list
 * \param second_list second nodes list
 *
 * \retval  0 nodes lists don't intersect
 * \retval  1 nodes lists intersect
*/
int
bridge_nodelist_intersects(bridge_nodelist_t* first_list,
			   bridge_nodelist_t* second_list);
/*!
 * @}
*/



/*!
 * \ingroup BRIDGE_COMMON
 * \brief Get number of tokens included in a string
 *
 * \param string input string
 * \param separators_list string containing allowed token 's separators
 * \param p_token_nb pointer on an integer that will be set to tokens quantity found in the string
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_common_string_get_tokens_quantity(char* string,char* separators_list,int* p_token_nb);



/*!
 * \ingroup BRIDGE_COMMON
 * \brief Get a specific token included in a string
 *
 * \param string input string
 * \param separators_list string containing allowed token 's separators
 * \param token id the id of the token in the list
 * \param p_token pointer on a string that will be set according to the token value (must be free later)
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int bridge_common_string_get_token(char* string,char* separators_list,int token_id,char** p_token);


/*!
 * \ingroup BRIDGE_COMMON
 * \brief Appends a char* giving a char* to append and an optional separator
 *
 * \param p_io_string pointer on the input char* that have to be appended
 * \param p_current_length pointer on the size_t that contains the current char* associated buffer (should be larger that string length)
 * \param inc_length incrementation step length
 * \param string2append char* that must be added
 * \param separator char* containing the separator
 *
 * \retval  0 operation successfully done
 * \retval -1 operation failed
*/
int
bridge_common_string_appends_and_extends(char** p_io_string,size_t* p_current_length,size_t inc_length,char* string2append,char* separator);

#endif
