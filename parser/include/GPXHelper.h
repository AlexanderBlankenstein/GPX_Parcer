/**
 * Created by: Alexander Blankenstein
 **/

#ifndef GPXHELPER_H
#define GPXHELPER_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <math.h>

#include "GPXParser.h"

/* ******************************* List helper functions *************************** */

void fillDoc(xmlNode* a_node, GPXdoc* tmpDoc, char* filename);

void createWaypoint(Waypoint* waypoint, xmlNode* a_node, xmlDocPtr doc);

void trim(char* str);

bool compareWaypointsBool(const void* first, const void* second);

bool compareRoutesBool(const void* first, const void* second);

bool compareTracksBool(const void* first, const void* second);

bool convertToXML(GPXdoc* doc, char* fileName);

bool validateXMLTree(char* fileName, char* gpxSchemaFile);

float haversine(float lat1, float lon1, float lat2, float lon2);

void dummyDelete(void* data);

int numPointsRoutes(const Route* rt);

int numPointsTracks(const Track* tr);

#endif