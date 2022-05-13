/**
 * Created by: Alexander Blankenstein
 **/

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <math.h>

#include "GPXHelper.h"
#include "GPXParser.h"

char subAttributes[7][1024] = { "name","desc","rtept","trkseg","trkpt","ele","time" };
char nodeAttributes[2][1024] = {"lat","lon" };

/** Function to initialize the list metadata and fills it with data from the parsed file. 
*@param Ptr- a pointer to a single node of the linked list
*@param Obj- the GPX document object to store data into
*@param Str- the filename to parse data from
**/
void fillDoc(xmlNode* a_node, GPXdoc* tmpDoc, char* filename) {
	xmlChar* sub;
	xmlDocPtr doc = xmlParseFile(filename);
	while (a_node) {
		if (a_node->type == XML_ELEMENT_NODE) {
			/* =========================================================   GPX   =========================================================== */

			if (strcmp((char*)a_node->name, "gpx") == 0) {
				xmlAttr* attr;
				//namespace
				char* cont = (char*)(a_node->ns->href);
				strcpy(((tmpDoc)->namespace), cont);

				for (attr = a_node->properties; attr != NULL; attr = attr->next)
				{
					xmlNode* value = attr->children;
					if (strcmp((char*)attr->name, "version") == 0) {
						char* valueData = (char*)(value->content);
						double data = strtod(valueData, NULL);
						((tmpDoc)->version) = data;
					}

					else if (strcmp((char*)attr->name, "creator") == 0) {
						char* creatorData = (char*)(value->content);
						int size = strlen(creatorData) + 1;
						((tmpDoc)->creator) = realloc((tmpDoc)->creator, sizeof(char) * size);
						strcpy(((tmpDoc)->creator), creatorData);
					}
				}
			}

			/* =========================================================   WPT   =========================================================== */

			else if (strcmp((char*)a_node->name, "wpt") == 0) {
				Waypoint* waypoint = malloc(sizeof(Waypoint));
				waypoint->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
				waypoint->latitude = 0.0;
				waypoint->longitude = 0.0;
				waypoint->name = malloc(2 * sizeof(char*));
				strcpy(waypoint->name, "");

				createWaypoint(waypoint, a_node, doc);
				insertBack(tmpDoc->waypoints, waypoint);
			}

			/* =========================================================   RTE   =========================================================== */

			else if (strcmp((char*)a_node->name, "rte") == 0) {
				Route* route = malloc(sizeof(Route));
				route->name = malloc(2 * sizeof(char*));
				strcpy(route->name, "");
				route->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
				route->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
				xmlNode* b_node = a_node->children;
				while (b_node)
				{
					for (int i = 0; i < 7; i++) {
						if ((!xmlStrcmp(b_node->name, (const xmlChar*)subAttributes[i]))) {
							sub = xmlNodeListGetString(doc, b_node->xmlChildrenNode, 1);
							if ((!xmlStrcmp(b_node->name, (const xmlChar*)"name"))) {
								char* nameData = (char*)(sub);
								int size = strlen(nameData) + 1;
								route->name = realloc(route->name, sizeof(char) * size);
								strcpy(route->name, nameData);
							}
							else if ((!xmlStrcmp(b_node->name, (const xmlChar*)"rtept"))) {
								Waypoint* waypoint = malloc(sizeof(Waypoint));
								waypoint->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
								waypoint->latitude = 0.0;
								waypoint->longitude = 0.0;
								waypoint->name = malloc(2 * sizeof(char*));
								strcpy(waypoint->name, "");

								createWaypoint(waypoint, b_node, doc);
								insertBack(route->waypoints, waypoint);
							}
							else{
								char* cont = (char*)(sub);
								GPXData* otherData = malloc(sizeof(GPXData) + sizeof(char) * (strlen(cont)+5));
								//trim(cont);
								strcpy((otherData)->name, (char*)b_node->name);
								strcpy(((otherData)->value), cont);
								insertBack(route->otherData, otherData);
							}
							xmlFree(sub);
						}
					}
					b_node = b_node->next;
				}
				insertBack(tmpDoc->routes, route);
			}

			/* =========================================================   TRK   =========================================================== */

			else if (strcmp((char*)a_node->name, "trk") == 0) {
				Track* track = malloc(sizeof(Track));
				track->name = malloc(2 * sizeof(char*));
				strcpy(track->name, "");
				track->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
				track->segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);

				xmlNode* b_node = a_node->children;
				while (b_node)
				{
					for (int i = 0; i < 7; i++) {
						if ((!xmlStrcmp(b_node->name, (const xmlChar*)subAttributes[i]))) {
							sub = xmlNodeListGetString(doc, b_node->xmlChildrenNode, 1);
							if ((!xmlStrcmp(b_node->name, (const xmlChar*)"name"))) {
								char* nameData = (char*)(sub);
								int size = strlen(nameData) +1;
								track->name = realloc(track->name, sizeof(char) * size);
								strcpy(track->name, nameData);
							}
							else if ((!xmlStrcmp(b_node->name, (const xmlChar*)"trkseg"))) {
								TrackSegment* trackSeg = malloc(sizeof(TrackSegment));
								trackSeg->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
								xmlNode* c_node = b_node->children;
								while (c_node) {
									if ((!xmlStrcmp(c_node->name, (const xmlChar*)"trkpt"))) {
										Waypoint* waypoint = malloc(sizeof(Waypoint));
										waypoint->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
										waypoint->latitude = 0.0;
										waypoint->longitude = 0.0;
										waypoint->name = malloc(2 * sizeof(char*));
										strcpy(waypoint->name, "");

										createWaypoint(waypoint, c_node, doc);
										insertBack(trackSeg->waypoints, waypoint);
									}
									c_node = c_node->next;
								}
								insertBack(track->segments, trackSeg);
							}
							else {
								char* cont = (char*)(sub);
								GPXData* otherData = malloc(sizeof(GPXData) + sizeof(char) * (strlen(cont) + 5));
								trim(cont);
								strcpy((otherData)->name, (char*)b_node->name);
								strcpy(((otherData)->value), cont);
								insertBack(track->otherData, otherData);
							}
							xmlFree(sub);
						}
					}
					b_node = b_node->next;
				}
				insertBack(tmpDoc->tracks, track);
			}
		}
		fillDoc(a_node->children, tmpDoc, filename);
		a_node = a_node->next;
	}
	xmlFreeDoc(doc);
}

/** Function to initialize a waypoint object and fill it with data from the linked list
*@return pointer to the waypoint
*@param Ptr- a pointer to a waypoint object
*@param Ptr- a pointer to a single node of the linked list
*@param Ptr- a pointer to an XML document
**/
void createWaypoint(Waypoint* waypoint, xmlNode* a_node, xmlDocPtr doc) {
	xmlChar* nod,* sub;
	for (int j = 0; j < 2; j++) {
		nod = xmlGetProp(a_node, (const xmlChar*)nodeAttributes[j]);
		if ((nod) != NULL) {
			if (strcmp(nodeAttributes[j], "lat") == 0) {
				char* valueData = (char*)nod;
				double data = strtod(valueData, NULL);
				waypoint->latitude = data;
			}
			else if (strcmp(nodeAttributes[j], "lon") == 0) {
				char* valueData = (char*)nod;
				double data = strtod(valueData, NULL);
				waypoint->longitude = data;
			}
		}
		xmlFree(nod);
	}
	xmlNode* b_node = a_node->children;
	while (b_node)
	{
		for (int i = 0; i < 7; i++) {
			if ((!xmlStrcmp(b_node->name, (const xmlChar*)subAttributes[i]))) {
				sub = xmlNodeListGetString(doc, b_node->xmlChildrenNode, 1);
				if (strcmp(subAttributes[i], "name") == 0) {
					char* creatorData = (char*)(sub);
					int size = strlen(creatorData) + 1;
					waypoint->name = realloc(waypoint->name, sizeof(char) * size);
					strcpy(waypoint->name, creatorData);
				}
				else{
					char* cont = (char*)(sub);
					GPXData* otherData = malloc(sizeof(GPXData) + sizeof(char) * (strlen(cont) + 5));

					strcpy((otherData)->name, (char*)b_node->name);
					strcpy(((otherData)->value), cont);
					insertBack(waypoint->otherData, otherData);
				}
				xmlFree(sub);
			}
		}
		b_node = b_node->next;
	}
}


/*
=========================================================   Trim function   ===========================================================
* Note that this can be left unused but I will keep it incase it becomes useful later on. I Found inspiration for this on Stack Overflow from aaron *
* link is as follows: https:// stackoverflow.com/questions/1726302/removing-spaces-from-a-string-in-c *
*/
void trim(char* str)
{
	int index = -1;
	int i;

	i = 0;
	while (str[i] != '\0')
	{
		if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
		{
			index = i;
		}
		i++;
	}
	str[index + 1] = '\0';
}

/* =========================================================   Compare Helper Function   =========================================================== */
/** Function to compare two waypoint objects
 *@pre Both Objects are not NULL
 *@post both objects have not been modified in any way
 *@return A Boolean based off the results
 *@param obj- two pointers to objects each
 **/
bool compareWaypointsBool(const void* first, const void* second) {
	Waypoint* tmp;
	bool toReturn = false;

	if (first == NULL || second == NULL) {
		return toReturn;
	}

	tmp = (Waypoint*)first;
	if (strcmp((char*)tmp->name, (char*)second) == 0) {
		toReturn = true;
	}

	return toReturn;
}

/** Function to compare two route objects
 *@pre Both Objects are not NULL
 *@post both objects have not been modified in any way
 *@return A Boolean based off the results
 *@param obj- two pointers to objects each
 **/
bool compareRoutesBool(const void* first, const void* second) {
	Route* tmp;
	bool toReturn = false;

	if (first == NULL || second == NULL) {
		return toReturn;
	}

	tmp = (Route*)first;
	if (strcmp((char*)tmp->name, (char*)second) == 0) {
		toReturn = true;
	}

	return toReturn;
}

/** Function to compare two track objects
 *@pre Both Objects are not NULL
 *@post both objects have not been modified in any way
 *@return A Boolean based off the results
 *@param obj- two pointers to objects each
 **/
bool compareTracksBool(const void* first, const void* second) {
	Track* tmp;
	bool toReturn = false;

	if (first == NULL || second == NULL) {
		return toReturn;
	}

	tmp = (Track*)first;
	if (strcmp((char*)tmp->name, (char*)second) == 0) {
		toReturn = true;
	}

	return toReturn;
}

/** Function to convert a GPX object to XML
 *@pre Doc Object is not NULL
 *@post Doc object have not been modified in any way
 *@return A Boolean based off the success
 *@param ptr- a pointer object that points to the GPX document
		str- the filename that the resulting XML should be called. 
 **/
bool convertToXML(GPXdoc* doc, char* fileName) {
	xmlDocPtr docPtr = NULL;       /* document pointer */
	xmlNodePtr root_node = NULL, wpt_node = NULL, rte_node = NULL, trk_node = NULL, rtept_node = NULL, trkpt_node = NULL, trkseg_node = NULL, child_node = NULL;/* node pointers */

	LIBXML_TEST_VERSION;

	/*
	 * Creates a new document, a node and set it as a root node
	 */

	if (doc != NULL) {
		/*
		=========================================================   GPX   ===========================================================
		*/

		docPtr = xmlNewDoc(BAD_CAST "1.0");
		root_node = xmlNewNode(NULL, BAD_CAST "gpx");
		xmlDocSetRootElement(docPtr, root_node);

		xmlNsPtr ns = xmlNewNs(root_node, BAD_CAST doc->namespace, NULL);
		xmlSetNs(root_node, ns);

		char* str = malloc(sizeof(char) * 16);
		sprintf(str, "%.1f", doc->version);
		xmlNewProp(root_node, BAD_CAST "version", BAD_CAST str);
		free(str);
		xmlNewProp(root_node, BAD_CAST "creator", BAD_CAST doc->creator);
		
		/*
		=========================================================   WPT   ===========================================================
		*/

		if (doc->waypoints != NULL) {
			ListIterator iter = createIterator(doc->waypoints);
			void* elem;
			while ((elem = nextElement(&iter)) != NULL) {
				Waypoint* tmpWpt = (Waypoint*)elem;
				wpt_node = xmlNewChild(root_node, NULL, BAD_CAST "wpt", NULL);
				if (tmpWpt->latitude != 0.0) {
					char* str = malloc(sizeof(char) * 16);
					sprintf(str, "%f", tmpWpt->latitude);
					xmlNewProp(wpt_node, BAD_CAST "lat", BAD_CAST str);
					free(str);
				}
				if (tmpWpt->longitude != 0.0) {
					char* str = malloc(sizeof(char) * 16);
					sprintf(str, "%f", tmpWpt->longitude);
					xmlNewProp(wpt_node, BAD_CAST "lon", BAD_CAST str);
					free(str);
				}
				if (strcmp(tmpWpt->name, "") != 0) {
					child_node = xmlNewChild(wpt_node, NULL, BAD_CAST "name", BAD_CAST tmpWpt->name);
					xmlAddChild(wpt_node, child_node);
				}
				if (tmpWpt->otherData != NULL) {
					ListIterator iterOD = createIterator(tmpWpt->otherData);
					void* elemOD;
					while ((elemOD = nextElement(&iterOD)) != NULL) {
						GPXData* tmpOD = (GPXData*)elemOD;
						child_node = xmlNewChild(wpt_node, NULL, BAD_CAST tmpOD->name, BAD_CAST tmpOD->value);
						xmlAddChild(wpt_node, child_node);
					}
				}
				xmlAddChild(root_node, wpt_node);
			}
		}

		/*
		=========================================================   RTE   ===========================================================
		*/

		if (doc->routes != NULL) {
			ListIterator iter = createIterator(doc->routes);
			void* elem;
			while ((elem = nextElement(&iter)) != NULL) {
				Route* tmpRte = (Route*)elem;
				rte_node = xmlNewChild(root_node, NULL, BAD_CAST "rte", NULL);
				if (strcmp(tmpRte->name, "") != 0) {
					child_node = xmlNewChild(rte_node, NULL, BAD_CAST "name", BAD_CAST tmpRte->name);
					xmlAddChild(rte_node, child_node);
				}
				if (tmpRte->otherData != NULL) {
					ListIterator iterOD = createIterator(tmpRte->otherData);
					void* elemOD;
					while ((elemOD = nextElement(&iterOD)) != NULL) {
						GPXData* tmpOD = (GPXData*)elemOD;
						child_node = xmlNewChild(rte_node, NULL, BAD_CAST tmpOD->name, BAD_CAST tmpOD->value);
						xmlAddChild(rte_node, child_node);
					}
				}
				if (tmpRte->waypoints != NULL) {
					ListIterator iter2 = createIterator(tmpRte->waypoints);
					void* elem2;
					while ((elem2 = nextElement(&iter2)) != NULL) {
						Waypoint* tmpWpt2 = (Waypoint*)elem2;
						rtept_node = xmlNewChild(rte_node, NULL, BAD_CAST "rtept", NULL);
						if (tmpWpt2->latitude != 0.0) {
							char* str = malloc(sizeof(char) * 16);
							sprintf(str, "%f", tmpWpt2->latitude);
							xmlNewProp(rtept_node, BAD_CAST "lat", BAD_CAST str);
							free(str);
						}
						if (tmpWpt2->longitude != 0.0) {
							char* str = malloc(sizeof(char) * 16);
							sprintf(str, "%f", tmpWpt2->longitude);
							xmlNewProp(rtept_node, BAD_CAST "lon", BAD_CAST str);
							free(str);
						}
						if (strcmp(tmpWpt2->name, "") != 0) {
							child_node = xmlNewChild(rtept_node, NULL, BAD_CAST "name", BAD_CAST tmpWpt2->name);
							xmlAddChild(rtept_node, child_node);
						}
						if (tmpWpt2->otherData != NULL) {
							ListIterator iterOD = createIterator(tmpWpt2->otherData);
							void* elemOD;
							while ((elemOD = nextElement(&iterOD)) != NULL) {
								GPXData* tmpOD = (GPXData*)elemOD;
								child_node = xmlNewChild(rtept_node, NULL, BAD_CAST tmpOD->name, BAD_CAST tmpOD->value);
								xmlAddChild(rtept_node, child_node);
							}
						}
						xmlAddChild(rte_node, rtept_node);
					}
				}
				xmlAddChild(root_node, rte_node);
			}
		}

		/*
		=========================================================   TRK   ===========================================================
		*/

		if (doc->tracks != NULL) {
			ListIterator iter = createIterator(doc->tracks);
			void* elem;
			while ((elem = nextElement(&iter)) != NULL) {
				Track* tmpTrk = (Track*)elem;
				trk_node = xmlNewChild(root_node, NULL, BAD_CAST "trk", NULL);
				if (strcmp(tmpTrk->name, "") != 0) {
					child_node = xmlNewChild(trk_node, NULL, BAD_CAST "name", BAD_CAST tmpTrk->name);
					xmlAddChild(trk_node, child_node);
				}
				if (tmpTrk->otherData != NULL) {
					ListIterator iterOD = createIterator(tmpTrk->otherData);
					void* elemOD;
					while ((elemOD = nextElement(&iterOD)) != NULL) {
						GPXData* tmpOD = (GPXData*)elemOD;
						child_node = xmlNewChild(trk_node, NULL, BAD_CAST tmpOD->name, BAD_CAST tmpOD->value);
						xmlAddChild(trk_node, child_node);
					}
				}
				if (tmpTrk->segments != NULL) {
					ListIterator iter2 = createIterator(tmpTrk->segments);
					void* elem2;
					while ((elem2 = nextElement(&iter2)) != NULL) {
						TrackSegment* tmpSeg = (TrackSegment*)elem2;
						trkseg_node = xmlNewChild(trk_node, NULL, BAD_CAST "trkseg", NULL);

						if (tmpSeg->waypoints != NULL) {
							ListIterator iter3 = createIterator(tmpSeg->waypoints);
							void* elem3;
							while ((elem3 = nextElement(&iter3)) != NULL) {
								Waypoint* tmpWpt3 = (Waypoint*)elem3;
								trkpt_node = xmlNewChild(trkseg_node, NULL, BAD_CAST "trkpt", NULL);
								if (tmpWpt3->latitude != 0.0) {
									char* str = malloc(sizeof(char) * 16);
									sprintf(str, "%f", tmpWpt3->latitude);
									xmlNewProp(trkpt_node, BAD_CAST "lat", BAD_CAST str);
									free(str);
								}
								if (tmpWpt3->longitude != 0.0) {
									char* str = malloc(sizeof(char) * 16);
									sprintf(str, "%f", tmpWpt3->longitude);
									xmlNewProp(trkpt_node, BAD_CAST "lon", BAD_CAST str);
									free(str);
								}
								if (strcmp(tmpWpt3->name, "") != 0) {
									child_node = xmlNewChild(trkpt_node, NULL, BAD_CAST "name", BAD_CAST tmpWpt3->name);
									xmlAddChild(trkpt_node, child_node);
								}
								if (tmpWpt3->otherData != NULL) {
									ListIterator iterOD = createIterator(tmpWpt3->otherData);
									void* elemOD;
									while ((elemOD = nextElement(&iterOD)) != NULL) {
										GPXData* tmpOD = (GPXData*)elemOD;
										child_node = xmlNewChild(trkpt_node, NULL, BAD_CAST tmpOD->name, BAD_CAST tmpOD->value);
										xmlAddChild(trkpt_node, child_node);
									}
								}
								xmlAddChild(trkseg_node, trkpt_node);
							}
						}
						xmlAddChild(trk_node, trkseg_node);
					}
				}
				xmlAddChild(root_node, trk_node);
			}
		}
	}
	int num = xmlSaveFormatFileEnc(fileName, docPtr, "UTF-8", 1);
	
	xmlFreeDoc(docPtr);

	if (num != -1)
	{
		return true;
	}
	return false;
}

/** Function to confirm the XML tree is correctly made
 *@pre Strings are not NULL
 *@return A Boolean based off the success
 *@param 
	str- a pointer object that points to the GPX schema
	str- the filename of the XML tree needing to be checked.  
 **/
bool validateXMLTree(char* fileName, char* gpxSchemaFile) {
	bool result = false;
	xmlDocPtr doc;
	xmlSchemaPtr schema = NULL;
	xmlSchemaParserCtxtPtr ctxt;

	xmlLineNumbersDefault(1);

	ctxt = xmlSchemaNewParserCtxt(gpxSchemaFile);

	xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stderr);
	schema = xmlSchemaParse(ctxt);
	xmlSchemaFreeParserCtxt(ctxt);

	doc = xmlReadFile(fileName, NULL, 0);

	if (doc == NULL) {
		fprintf(stderr, "Could not parse file\n");
	}
	else {
		xmlSchemaValidCtxtPtr ctxt;
		int ret;

		ctxt = xmlSchemaNewValidCtxt(schema);
		xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stderr);
		ret = xmlSchemaValidateDoc(ctxt, doc);
		if (ret == 0)
		{
			printf("file validates\n");
			result = true;
		}
		else if (ret > 0)
		{
			printf("file fails to validate\n");
		}
		else
		{
			printf("file validation generated an internal error\n");
		}
		xmlSchemaFreeValidCtxt(ctxt);
		xmlFreeDoc(doc);
	}

	if (schema != NULL) {
		xmlSchemaFree(schema);
	}

	xmlSchemaCleanupTypes();
	xmlCleanupParser();
	xmlMemoryDump();

	return result;
}

/** Function to calculate the haversine between two points
 *@pre floats are not NULL
 *@return A float value representing the distance between. 
 *@param Float- of two latitudes and 2 longitudes
 **/
float haversine(float lat1, float lon1, float lat2, float lon2) {
	double pi = 3.141592653589793;
	int rad = 6371e3;
	double lat1Tmp = (lat1) * (pi / 180);
	double lat2Tmp = (lat2) * (pi / 180);
	double differenceLon = (lon2 - lon1) * (pi / 180);
	double differenceLat = (lat2 - lat1) * (pi / 180);
	double a = sin(differenceLat / 2) * sin(differenceLat / 2) + 
		cos(lat1Tmp) * cos(lat2Tmp) * 
		sin(differenceLon / 2) * sin(differenceLon / 2);

	double c = 2 * atan2(sqrt(a), sqrt(1 - a));
	float distance = rad * c;
	return distance;
}

/** Function used since we always need to provide a delte function even when we dont wish to delete. 
 *@param ptr- any data pointer. 
 **/
void dummyDelete(void* data) {
	//do nothing
}

/** Function to count the number of points withing a route
 *@pre route pointer is not NULL
 *@return An integer value of number of points. 
 *@param ptr- the route pointer
 **/
int numPointsRoutes(const Route* rt) {
	ListIterator iter;
	iter.current = rt->waypoints->head;
	int num = 0;
	void* elem;

	if (rt != NULL) {
		while ((elem = nextElement(&iter)) != NULL) {
			num = num + 1;
		}
	}

	return num;
}

/** Function to count the number of points withing a track
 *@pre track pointer is not NULL
 *@return An integer value of number of points. 
 *@param ptr- the track pointer
 **/
int numPointsTracks(const Track* tr) {
	ListIterator iter = createIterator(tr->segments);
	int num = 0;
	void* elem;

	if (tr != NULL) {
		while ((elem = nextElement(&iter)) != NULL) {
			TrackSegment* tmpTrSeg = (TrackSegment*)elem;
			ListIterator iter2 = createIterator(tmpTrSeg->waypoints);
			void* elem2;

			while ((elem2 = nextElement(&iter2)) != NULL) {
				num = num + 1;
			}
		}
	}

	return num;
}