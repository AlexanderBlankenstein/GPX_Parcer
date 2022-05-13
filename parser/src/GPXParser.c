/**
 * Created by: Alexander Blankenstein
 **/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>

#include "GPXParser.h"
#include "assert.h"
#include "LinkedListAPI.h"
#include "GPXHelper.h"

/** Function to create an GPX object based on the contents of an GPX file.
 *@pre File name cannot be an empty string or NULL.
       File represented by this name must exist and must be readable.
 *@post Either:
        A valid GPXdoc has been created and its address was returned
        or
        An error occurred, and NULL was returned
 *@return the pinter to the new struct or NULL
 *@param fileName - a string containing the name of the GPX file
**/
GPXdoc* createGPXdoc(char* fileName) {
    xmlDoc* doc = NULL;
    
    if (fileName == NULL) {
        return NULL;
    }

    char* point;
    if ((point = strrchr(fileName, '.')) != NULL) {
        if (strcmp(point, ".gpx") == 0) {
            LIBXML_TEST_VERSION
            doc = xmlReadFile(fileName, NULL, 64);
        }
        else {
            return NULL;
        }
    }
    else {
        xmlFreeDoc(doc);
        return NULL;
    }
    if (doc == NULL) {
        return NULL;
    }
    
    //Init data tree
    GPXdoc* tmpDoc = malloc(sizeof(GPXdoc));
    tmpDoc->version = 0.0;
    tmpDoc->creator = malloc(2 * sizeof(char*));
    strcpy(tmpDoc->creator, "");
    tmpDoc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    tmpDoc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    tmpDoc->tracks = initializeList(&trackToString, &deleteTrack, compareTracks);

    xmlNode* root_element = xmlDocGetRootElement(doc);
    fillDoc(root_element,tmpDoc, fileName);

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return tmpDoc;
}

/** Function to create a string representation of an GPX object.
 *@pre GPX object exists, is not null, and is valid
 *@post GPX has not been modified in any way, and a string representing the GPX contents has been created
 *@return a string contaning a humanly readable representation of an GPX object
 *@param obj - a pointer to an GPX struct
**/
char* GPXdocToString(GPXdoc* doc) {
    char* docString = (char*)malloc(sizeof(char));
    int size = 0;
    strcpy(docString, "");

    if (doc != NULL) {
        if (doc->version != 0.0) {
            char* versionStr = malloc(sizeof(char)*30);
            sprintf(versionStr, "Version : %.1f", doc->version);
            size = strlen(docString) + strlen(versionStr) + 2;
            docString = realloc(docString, (sizeof(char)*size));
            strcat(docString, versionStr);
            strcat(docString, "\n");
            free(versionStr);
        }
        if (doc->creator != NULL) {
            char* creatorStr = doc->creator;
            size = strlen(docString) + strlen("Creator: ") + strlen(creatorStr) + 2;
            docString = realloc(docString, (sizeof(char)*size));
            strcat(docString, "Creator: ");
            strcat(docString, creatorStr);
            strcat(docString, "\n");
            //free(creatorStr);
        }
        if (doc->namespace != NULL) {
            char* nsStr = malloc(sizeof(char) * 261);
            sprintf(nsStr, "Namespace : %s", doc->namespace);
            size = strlen(docString) + strlen(nsStr) + 2;
            docString = realloc(docString, (sizeof(char) * size));
            strncat(docString, nsStr, strlen(nsStr));
            strcat(docString, "\n");
            free(nsStr);
        }
        if (doc->waypoints != NULL) {
            char* waypointStr = toString(doc->waypoints);
            size = strlen(docString) + strlen("----<Waypoint>----") + strlen(waypointStr) + 2;
            docString = realloc(docString, (sizeof(char)*size));
            strcat(docString, "----<Waypoint>----");
            strcat(docString, waypointStr);
            strcat(docString, "\n");
            free(waypointStr);
        }
        if (doc->routes != NULL) {
            char* routeStr = toString(doc->routes);
            size = strlen(docString) + strlen("----<Routes>----") + strlen(routeStr) + 2;
            docString = realloc(docString, (sizeof(char)*size));
            strcat(docString, "----<Routes>----");
            strcat(docString, routeStr);
            strcat(docString, "\n");
            free(routeStr);
        }
        if (doc->tracks != NULL) {
            char* trackStr = toString(doc->tracks);
            size = strlen(docString) + strlen("----<Tracks>----") + strlen(trackStr) + 2;
            docString = realloc(docString, (sizeof(char) * size));
            strcat(docString, "----<Tracks>----");
            strcat(docString, trackStr);
            strcat(docString, "\n");
            free(trackStr);
        }
    }
    else {
        strcat(docString, "GPX Doc is empty!\n");
    }

    return docString;
}

/** Function to delete doc content and free all the memory.
 *@pre GPX object exists, is not null, and has not been freed
 *@post GPX object had been freed
 *@return none
 *@param obj - a pointer to an GPX struct
**/
void deleteGPXdoc(GPXdoc* doc) {
    if (doc == NULL) {
        return;
    }

    if (doc != NULL) {
        if (doc->creator != NULL) {
            free(doc->creator);
        }
        if (doc->version != 0.0) {
            doc->version = 0.0;
        }
        if (doc->waypoints != NULL) {
            freeList(doc->waypoints);
        }
        if (doc->routes != NULL) {
            freeList(doc->routes);
        }
        if (doc->tracks != NULL) {
            freeList(doc->tracks);
        }
    }
    free(doc);
}

/* For the five "get..." functions below, return the count of specified entities from the file.
They all share the same format and only differ in what they have to count.

 *@pre GPX object exists, is not null, and has not been freed
 *@post GPX object has not been modified in any way
 *@return the number of entities in the GPXdoc object
 *@param obj - a pointer to an GPXdoc struct
 */
 
 //Total number of waypoints in the GPX file
int getNumWaypoints(const GPXdoc* doc) {
    if (doc == NULL) {
        return 0;
    }

    ListIterator iter = createIterator(doc->waypoints);
    int num = 0;
    void* elem;

    if (doc != NULL) {
        while ((elem = nextElement(&iter)) != NULL) {
            num = num + 1;
        }
    }

    return num;
}

//Total number of routes in the GPX file
int getNumRoutes(const GPXdoc* doc) {
    if (doc == NULL) {
        return 0;
    }
    ListIterator iter = createIterator(doc->routes);
    int num = 0;
    void* elem;

    if (doc != NULL) {
        while ((elem = nextElement(&iter)) != NULL) {
            num = num + 1;
        }
    }

    return num;
}

//Total number of tracks in the GPX file
int getNumTracks(const GPXdoc* doc) {
    if (doc == NULL) {
        return 0;
    }
    ListIterator iter = createIterator(doc->tracks);
    int num = 0;
    void* elem;

    if (doc != NULL) {
        while ((elem = nextElement(&iter)) != NULL) {
            num = num + 1;
        }
    }

    return num;
}

//Total number of segments in all tracks in the document
int getNumSegments(const GPXdoc* doc) {
    if (doc == NULL) {
        return 0;
    }
    ListIterator iter = createIterator(doc->tracks);
    int num = 0;
    void* elem;

    if (doc != NULL) {
        while ((elem = nextElement(&iter)) != NULL) {
            Track* tmpTRK = (Track*)elem;
            ListIterator iter2 = createIterator(tmpTRK->segments);
            void* elem2;

            while ((elem2 = nextElement(&iter2)) != NULL) {
                num = num + 1;
            }
        }
    }

    return num;
}

//Total number of GPXData elements in the document
int getNumGPXData(const GPXdoc* doc) {
    if (doc == NULL) {
        return 0;
    }
    ListIterator iter = createIterator(doc->waypoints);
    int num = 0;
    void* elem;

    if (doc != NULL) {
        while ((elem = nextElement(&iter)) != NULL) {
            Waypoint* tmpWPT = (Waypoint*)elem;
            ListIterator iter2 = createIterator(tmpWPT->otherData);
            void* elem2;

            while ((elem2 = nextElement(&iter2)) != NULL) {
                num = num + 1;
            }

            if (strcmp(tmpWPT->name, "") != 0) {
                num = num + 1;
            }
        }
    }

    iter = createIterator(doc->routes);

    if (doc != NULL) {
        while ((elem = nextElement(&iter)) != NULL) {
            Route* tmpRTE = (Route*)elem;
            ListIterator iter2 = createIterator(tmpRTE->otherData);
            void* elem2;

            while ((elem2 = nextElement(&iter2)) != NULL) {
                num = num + 1;
            }

            ListIterator iter3 = createIterator(tmpRTE->waypoints);
            void* elem3;

            while ((elem3 = nextElement(&iter3)) != NULL) {
                Waypoint* tmpWPT = (Waypoint*)elem3;
                ListIterator iter4 = createIterator(tmpWPT->otherData);
                void* elem4;

                while ((elem4 = nextElement(&iter4)) != NULL) {
                    num = num + 1;
                }

                if (strcmp(tmpWPT->name, "") != 0) {
                    num = num + 1;
                }
            }

            if (strcmp(tmpRTE->name, "") != 0) {
                num = num + 1;
            }
        }
    }

    iter = createIterator(doc->tracks);

    if (doc != NULL) {
        while ((elem = nextElement(&iter)) != NULL) {
            Track* tmpTRK = (Track*)elem;
            ListIterator iter2 = createIterator(tmpTRK->otherData);
            void* elem2;

            while ((elem2 = nextElement(&iter2)) != NULL) {
                num = num + 1;
            }

            ListIterator iter3 = createIterator(tmpTRK->segments);
            void* elem3;

            while ((elem3 = nextElement(&iter3)) != NULL) {
                TrackSegment* tmpTS = (TrackSegment*)elem3;
                ListIterator iter4 = createIterator(tmpTS->waypoints);
                void* elem4;

                while ((elem4 = nextElement(&iter4)) != NULL) {
                    Waypoint* tmpWPT = (Waypoint*)elem4;
                    ListIterator iter5 = createIterator(tmpWPT->otherData);
                    void* elem5;

                    while ((elem5 = nextElement(&iter5)) != NULL) {
                        num = num + 1;
                    }

                    if (strcmp(tmpWPT->name, "") != 0) {
                        num = num + 1;
                    }
                }
            }

            if (strcmp(tmpTRK->name, "") != 0) {
                num = num + 1;
            }
        }
    }

    return num;
}

/* Function that returns a waypoint with the given name.  If more than one exists, return the first one.  

 *@pre GPX object exists, is not null, and has not been freed
 *@post GPX object has not been modified in any way
 *@return NULL if the waypoint does not exist
 *@param 
	obj - a pointer to an GPXdoc struct, 
	name - a string containing the name of the waypoint
 */
Waypoint* getWaypoint(const GPXdoc* doc, char* name) {
    if (doc == NULL || name == NULL)
    {
        return NULL;
    }

    ListIterator itr = createIterator(doc->waypoints);
    void* data = nextElement(&itr);
    while (data != NULL)
    {
        if (compareWaypointsBool(data, name))
            return data;

        data = nextElement(&itr);
    }

    return NULL;
}


/* Function that returns a track with the given name.  If more than one exists, return the first one. 

 *@pre GPX object exists, is not null, and has not been freed
 *@post GPX object has not been modified in any way
 *@return NULL if the track does not exist
 *@param 
	obj - a pointer to an GPXdoc struct, 
	name - a string containing the name of the track
 */
Track* getTrack(const GPXdoc* doc, char* name) {
    if (doc == NULL)
    {
        return NULL;
    }

    ListIterator itr = createIterator(doc->tracks);
    void* data = nextElement(&itr);
    while (data != NULL)
    {
        if (compareTracksBool(data, name))
            return data;

        data = nextElement(&itr);
    }
    return NULL;
}
/* Function that returns a route with the given name.  If more than one exists, return the first one. 

 *@pre GPX object exists, is not null, and has not been freed
 *@post GPX object has not been modified in any way
 *@return NULL if the route does not exist
 *@param 
	obj - a pointer to an GPXdoc struct
	name - a string containing the name of the route
 */
Route* getRoute(const GPXdoc* doc, char* name) {
    if (doc == NULL)
    {
        return NULL;
    }

    ListIterator itr = createIterator(doc->routes);
    void* data = nextElement(&itr);
    while (data != NULL)
    {
        if (compareRoutesBool(data, name))
            return data;

        data = nextElement(&itr);
    }
    return NULL;
}

/** Function to create an GPX object based on the contents of an GPX file.
 * This function must validate the XML tree generated by libxml against a GPX schema file
 * before attempting to traverse the tree and create an GPXdoc struct
 *@pre File name cannot be an empty string or NULL.
       File represented by this name must exist and must be readable.
 *@post Either:
        A valid GPXdoc has been created and its address was returned
        or
        An error occurred, and NULL was returned
 *@return the pinter to the new struct or NULL
 *@param gpxSchemaFile - the name of a schema file
 *@param fileName - a string containing the name of the GPX file
**/
GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile) {
    if (fileName == NULL || gpxSchemaFile == NULL) {
        return NULL;
    }

    xmlDoc* doc = NULL;
    bool valid = false;

    GPXdoc* tmpDoc = malloc(sizeof(GPXdoc));
    tmpDoc->version = 0.0;
    tmpDoc->creator = malloc(2 * sizeof(char*));
    strcpy(tmpDoc->creator, "");
    tmpDoc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    tmpDoc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    tmpDoc->tracks = initializeList(&trackToString, &deleteTrack, compareTracks);

    char* point;
    if ((point = strrchr(fileName, '.')) != NULL) {
        if (strcmp(point, ".gpx") == 0) {
            valid = validateXMLTree(fileName, gpxSchemaFile);
        }
        else {
            xmlFreeDoc(doc);
            return NULL;
        }
    }
    else {
        xmlFreeDoc(doc);
        return NULL;
    }

    if (valid)
    {
        LIBXML_TEST_VERSION
        doc = xmlReadFile(fileName, NULL, 64);
        
        xmlNode* root_element = xmlDocGetRootElement(doc);
        fillDoc(root_element, tmpDoc, fileName);
    }

    xmlFreeDoc(doc);
    return tmpDoc;
}

/** Function to validating an existing a GPXobject object against a GPX schema file
 *@pre
    GPXdoc object exists and is not NULL
    schema file name is not NULL/empty, and represents a valid schema file
 *@post GPXdoc has not been modified in any way
 *@return the boolean aud indicating whether the GPXdoc is valid
 *@param doc - a pointer to a GPXdoc struct
 *@param gpxSchemaFile - the name of a schema file
 **/
bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile) {
    if (doc == NULL || gpxSchemaFile == NULL)
    {
        return false;
    }

    char* fileName = "Validationfile.gpx";

    bool valid = convertToXML(doc, fileName);

    if (valid) {
        valid = validateXMLTree(fileName, gpxSchemaFile);
    }

    return valid;
}

/** Function to writing a GPXdoc into a file in GPX format.
 *@pre
    GPXdoc object exists, is valid, and and is not NULL.
    fileName is not NULL, has the correct extension
 *@post GPXdoc has not been modified in any way, and a file representing the
    GPXdoc contents in GPX format has been created
 *@return a boolean value indicating success or failure of the write
 *@param
    doc - a pointer to a GPXdoc struct
    fileName - the name of the output file
 **/
bool writeGPXdoc(GPXdoc* doc, char* fileName) {
    if (doc == NULL || fileName == NULL)
    {
        return false;
    }
    char* point;
    if ((point = strrchr(fileName, '.')) != NULL) {
        if (strcmp(point, ".gpx") != 0) {
            return false;
        }
    }

    bool result = convertToXML(doc, fileName);

    return result;
}

/** Function that returns the length of a Route
 *@pre Route object exists, is not null, and has not been freed
 *@post Route object had been freed
 *@return length of the route in meters
 *@param rt - a pointer to a Route struct
**/
float getRouteLen(const Route* rt) {
    float toReturn = 0.0;

    if (rt == NULL || rt->waypoints == NULL) {
        return toReturn;
    }
    else {
        Waypoint* tmpWpt2 = NULL;
        ListIterator iter = createIterator(rt->waypoints);
        void* wpt;

        while ((wpt = nextElement(&iter)) != NULL) {
            Waypoint* tmpWpt1 = (Waypoint*)wpt;

            if (tmpWpt2 != NULL)
            {
                toReturn = toReturn + haversine(tmpWpt1->latitude, tmpWpt1->longitude, tmpWpt2->latitude, tmpWpt2->longitude);
            }
            tmpWpt2 = tmpWpt1;
        }
    }

    return toReturn;
}

/** Function that returns the length of a Track
 *@pre Track object exists, is not null, and has not been freed
 *@post Track object had been freed
 *@return length of the track in meters
 *@param tr - a pointer to a Track struct
**/
float getTrackLen(const Track* tr) {
    float toReturn = 0.0;

    if (tr == NULL) {
        return toReturn;
    }
    else {
        Waypoint* tmpWpt2 = NULL;

        ListIterator iter = createIterator(tr->segments);
        void* seg;

        while ((seg = nextElement(&iter)) != NULL) {
            TrackSegment* tmpSeg = (TrackSegment*)seg;

            ListIterator iter2 = createIterator(tmpSeg->waypoints);
            void* wpt;

            while ((wpt = nextElement(&iter2)) != NULL) {
                Waypoint* tmpWpt1 = (Waypoint*)wpt;
                
                if (tmpWpt2 != NULL)
                {
                    toReturn = toReturn + haversine(tmpWpt1->latitude, tmpWpt1->longitude, tmpWpt2->latitude, tmpWpt2->longitude);
                }
                tmpWpt2 = tmpWpt1;
            }
        }
    }

    return toReturn;
}

/** Function that rounds the length of a track or a route to the nearest 10m
 *@pre Length is not negative
  *@return length rounded to the nearest 10m
 *@param len - length
**/
float round10(float len) {
    float toReturn = 0.0;

    toReturn = (10 * round((len / 10)));

    return toReturn;
}

/** Function that returns the number routes with the specified length, using the provided tolerance
 * to compare route lengths
 *@pre GPXdoc object exists, is not null
 *@post GPXdoc object exists, is not null, has not been modified
 *@return the number of routes with the specified length
 *@param doc - a pointer to a GPXdoc struct
 *@param len - search route length
 *@param delta - the tolerance used for comparing route lengths
**/
int numRoutesWithLength(const GPXdoc* doc, float len, float delta) {
    int toReturn = 0;
    if (doc == NULL || len < 0 || delta < 0)
    {
        return toReturn;
    }

    ListIterator iter = createIterator(doc->routes);
    void* elem;
    while ((elem = nextElement(&iter)) != NULL) {
        Route* tmpRte = (Route*)elem;

        float length = getRouteLen(tmpRte);

        float dif = length - len;
        if (dif < 0)
        {
            dif = dif * (-1);
        }
        if (dif <= delta)
        {
            toReturn = toReturn + 1;
        }
    }

    return toReturn;
}


/** Function that returns the number tracks with the specified length, using the provided tolerance
 * to compare track lengths
 *@pre GPXdoc object exists, is not null
 *@post GPXdoc object exists, is not null, has not been modified
 *@return the number of tracks with the specified length
 *@param doc - a pointer to a GPXdoc struct
 *@param len - search track length
 *@param delta - the tolerance used for comparing track lengths
**/
int numTracksWithLength(const GPXdoc* doc, float len, float delta) {
    int toReturn = 0;
    if (doc == NULL || len < 0 || delta < 0)
    {
        return toReturn;
    }

    ListIterator iter = createIterator(doc->tracks);
    void* elem;
    while ((elem = nextElement(&iter)) != NULL) {
        Track* tmpTrk = (Track*)elem;

        float length = getTrackLen(tmpTrk);

        float dif = length - len;
        if (dif < 0)
        {
            dif = dif * (-1);
        }
        if (dif <= delta)
        {
            toReturn = toReturn + 1;
        }
    }

    return toReturn;
}

/** Function that checks if the current route is a loop
 *@pre Route object exists, is not null
 *@post Route object exists, is not null, has not been modified
 *@return true if the route is a loop, false otherwise
 *@param route - a pointer to a Route struct
 *@param delta - the tolerance used for comparing distances between start and end points
**/
bool isLoopRoute(const Route* route, float delta) {
    /*
    For a route to form a loop, it must:
    -Have at least 4 waypoints
    -Have the distance of less than delta between the first and last points.
    */
    bool result = false;
    if (route == NULL || delta < 0)
    {
        return result;
    }
    else {
        ListIterator iter = createIterator(route->waypoints);
        int numRte = 0;
        void* elem;

        while ((elem = nextElement(&iter)) != NULL) {
            numRte = numRte + 1;
        }

        Waypoint* firstWpt = getFromFront(route->waypoints);
        Waypoint* lastWpt = getFromBack(route->waypoints);
        int len = haversine(firstWpt->latitude, firstWpt->longitude, lastWpt->latitude, lastWpt->longitude);

        if (numRte >= 4 && len <= delta)
        {
            result = true;
        }
    }
    return result;
}


/** Function that checks if the current track is a loop
 *@pre Track object exists, is not null
 *@post Track object exists, is not null, has not been modified
 *@return true if the track is a loop, false otherwise
 *@param track - a pointer to a Track struct
 *@param delta - the tolerance used for comparing distances between start and end points
**/
bool isLoopTrack(const Track* tr, float delta) {
    bool result = false;
    if (tr == NULL || delta < 0)
    {
        return result;
    }
    else {
        bool reqLen = false;

        ListIterator iter = createIterator(tr->segments);
        void* seg;

        while ((seg = nextElement(&iter)) != NULL) {
            TrackSegment* tmpSeg = (TrackSegment*)seg;
            int numRte = 0;

            ListIterator iter2 = createIterator(tmpSeg->waypoints);
            void* wpt;

            while ((wpt = nextElement(&iter2)) != NULL) {
                numRte = numRte + 1;
            }

            if (numRte >= 4)
            {
                reqLen = true;
            }
        }

        TrackSegment* firstSeg = getFromFront(tr->segments);
        TrackSegment* lastSeg = getFromBack(tr->segments);

        Waypoint* firstWpt = getFromFront(firstSeg->waypoints);
        Waypoint* lastWpt = getFromBack(lastSeg->waypoints);

        int len = haversine(firstWpt->latitude, firstWpt->longitude, lastWpt->latitude, lastWpt->longitude);

        if (reqLen && len <= delta)
        {
            result = true;
        }
    }
    return result;
}


/** Function that returns all routes between the specified start and end locations
 *@pre GPXdoc object exists, is not null
 *@post GPXdoc object exists, is not null, has not been modified
 *@return a list of Route structs that connect the given sets of coordinates
 *@param doc - a pointer to a GPXdoc struct
 *@param sourceLat - latitude of the start location
 *@param sourceLong - longitude of the start location
 *@param destLat - latitude of the destination location
 *@param destLong - longitude of the destination location
 *@param delta - the tolerance used for comparing distances between waypoints
*/
List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {
    GPXdoc* tmpDoc = malloc(sizeof(GPXdoc));
    tmpDoc->routes = initializeList(&routeToString, &dummyDelete, &compareRoutes);

    bool hasContent = false;

    //see if source loaction exists. 
    //float sourceDist = 0.0;
    //float destDist = 0.0;
    //bool hasSource = false;

    if (doc == NULL || delta < 0) {
        return NULL;
    }
    else {
        ListIterator iter = createIterator(doc->routes);
        void* rte;

        while ((rte = nextElement(&iter)) != NULL) {
            Route* tmpRte = (Route*)rte;

            Waypoint* firstWpt = getFromFront(tmpRte->waypoints);
            Waypoint* lastWpt = getFromBack(tmpRte->waypoints);

            float sourceDist = haversine(firstWpt->latitude, firstWpt->longitude, sourceLat, sourceLong);
            float destDist = haversine(lastWpt->latitude, lastWpt->longitude, destLat, destLong);

            if (sourceDist <= delta && destDist <= delta)
            {
                insertBack(tmpDoc->routes, tmpRte);
                hasContent = true;
            }
        }
    }

    if (hasContent) {
        return tmpDoc->routes;
    }
    else {
        return NULL;
    }
}

/** Function that returns all Tracks between the specified start and end locations
 *@pre GPXdoc object exists, is not null
 *@post GPXdoc object exists, is not null, has not been modified
 *@return a list of Track structs that connect the given sets of coordinates
 *@param doc - a pointer to a GPXdoc struct
 *@param sourceLat - latitude of the start location
 *@param sourceLong - longitude of the start location
 *@param destLat - latitude of the destination location
 *@param destLong - longitude of the destination location
 *@param delta - the tolerance used for comparing distances between waypoints
*/
List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {
    GPXdoc* tmpDoc = malloc(sizeof(GPXdoc));
    tmpDoc->tracks = initializeList(&trackToString, &dummyDelete, compareTracks);

    bool hasContent = false;

    if (doc == NULL || delta < 0) {
        return NULL;
    }
    else {
        ListIterator iter = createIterator(doc->tracks);
        void* trk;

        while ((trk = nextElement(&iter)) != NULL) {
            Track* tmpTrk = (Track*)trk;

            TrackSegment* firstSeg = getFromFront(tmpTrk->segments);
            TrackSegment* lastSeg = getFromBack(tmpTrk->segments);

            Waypoint* firstWpt = getFromFront(firstSeg->waypoints);
            Waypoint* lastWpt = getFromBack(lastSeg->waypoints);

            float sourceDist = haversine(firstWpt->latitude, firstWpt->longitude, sourceLat, sourceLong);
            float destDist = haversine(lastWpt->latitude, lastWpt->longitude, destLat, destLong);

            if (sourceDist <= delta && destDist <= delta)
            {
                insertBack(tmpDoc->tracks, tmpTrk);
                hasContent = true;
            }
        }
    }

    if (hasContent) {
        return tmpDoc->tracks;
    }
    else {
        return NULL;
    }
}

/** Function to converting a Track into a JSON string
 *@pre Track is not NULL
 *@post Track has not been modified in any way
 *@return A string in JSON format
 *@param event - a pointer to a Track struct
 **/
char* trackToJSON(const Track* tr) {
    char* json = malloc(sizeof(char));
    strcpy(json, "");
    int size = 0;

    if (tr == NULL || tr->segments == NULL)
    {
        size = strlen(json) + strlen("{}");
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "{}");
    }
    else
    {
        int numPoints = numPointsTracks(tr);

        char* nameStr = tr->name;
        size = strlen(json) + strlen("{\"name\":") + strlen(nameStr) + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "{\"name\":\"");
        strcat(json, nameStr);

        char* numStr = malloc(sizeof(char) * 30);
        sprintf(numStr, "\",\"numPoints\":%d", numPoints);
        size = strlen(json) + strlen(numStr) + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, numStr);
        free(numStr);

        char* num2Str = malloc(sizeof(char) * 30);
        float num = getTrackLen(tr);
        num = round10(num);
        sprintf(num2Str, ",\"len\":%.1f", num);
        size = strlen(json) + strlen(num2Str) + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, num2Str);
        free(num2Str);

        char* loopStr = "false";
        if (isLoopTrack(tr,10.0))
        {
            loopStr = "true";
        }
        size = strlen(json) + strlen(",\"loop\":%s}") + strlen(loopStr) + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, ",\"loop\":");
        strcat(json, loopStr);
        strcat(json, "}");
    }
    return json;
}

/** Function to converting a Route into a JSON string
 *@pre Route is not NULL
 *@post Route has not been modified in any way
 *@return A string in JSON format
 *@param event - a pointer to a Route struct
 **/
char* routeToJSON(const Route* rt) {
    char* json = malloc(sizeof(char));
    strcpy(json, "");
    int size = 0;

    if (rt == NULL || rt->waypoints == NULL)
    {
        size = strlen(json) + strlen("{}");
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "{}");
    }
    else
    {
        int numPoints = numPointsRoutes(rt);

        char* nameStr = rt->name;
        size = strlen(json) + strlen("{\"name\":") + strlen(nameStr) + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "{\"name\":\"");
        strcat(json, nameStr);

        char* numStr = malloc(sizeof(char) * 30);
        sprintf(numStr, "\",\"numPoints\":%d", numPoints);
        size = strlen(json) + strlen(numStr) + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, numStr);
        free(numStr);

        char* num2Str = malloc(sizeof(char) * 30);
        float num = getRouteLen(rt);
        num = round10(num);
        sprintf(num2Str, ",\"len\":%.1f", num);
        size = strlen(json) + strlen(num2Str) + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, num2Str);
        free(num2Str);

        char* loopStr = "false";
        if (isLoopRoute(rt,10.0))
        {
            loopStr = "true";
        }
        size = strlen(json) + strlen(",\"loop\":%s}") + strlen(loopStr) + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, ",\"loop\":");
        strcat(json, loopStr);
        strcat(json, "}");
    }

    return json;
}

/** Function to converting a list of Route structs into a JSON string
 *@pre Route list is not NULL
 *@post Route list has not been modified in any way
 *@return A string in JSON format
 *@param event - a pointer to a List struct
 **/
char* routeListToJSON(const List* list) {
    char* json = malloc(sizeof(char));
    strcpy(json, "");
    int size = 0;
    bool notFirst = false;

    if (list == NULL)
    {
        size = strlen(json) + strlen("[]");
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "[]");
    }
    else 
    {
        size = strlen(json) + strlen("[") + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "[");

        ListIterator iter;

        iter.current = list->head;
        void* rte;

        while ((rte = nextElement(&iter)) != NULL) {
            Route* tmpRte = (Route*)rte;

            char* jsonRteStr = routeToJSON(tmpRte);
            size = strlen(json) + strlen(jsonRteStr) + 2;
            json = realloc(json, (sizeof(char) * size));
            if (notFirst)
            {
                strcat(json, ",");
            }
            strcat(json, jsonRteStr);
            notFirst = true;
        }

        size = strlen(json) + strlen("]") + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "]");
    }

    return json;
}

/** Function to converting a list of Track structs into a JSON string
 *@pre Track list is not NULL
 *@post Track list has not been modified in any way
 *@return A string in JSON format
 *@param event - a pointer to a List struct
 **/
char* trackListToJSON(const List* list) {
    char* json = malloc(sizeof(char));
    strcpy(json, "");
    int size = 0;
    bool notFirst = false;

    if (list == NULL)
    {
        size = strlen(json) + strlen("[]");
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "[]");
    }
    else
    {
        size = strlen(json) + strlen("[") + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "[");
        
        ListIterator iter;

        iter.current = list->head;
        void* trk;

        while ((trk = nextElement(&iter)) != NULL) {
            Track* tmpTrk = (Track*)trk;

            char* jsonTrkStr = trackToJSON(tmpTrk);
            size = strlen(json) + strlen(jsonTrkStr) + 2;
            json = realloc(json, (sizeof(char) * size));
            if (notFirst)
            {
                strcat(json, ",");
            }
            strcat(json, jsonTrkStr);
            notFirst = true;
        }

        size = strlen(json) + strlen("]") + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "]");
    }

    return json;
}

/** Function to converting a GPXdoc into a JSON string
 *@pre GPXdoc is not NULL
 *@post GPXdoc has not been modified in any way
 *@return A string in JSON format
 *@param event - a pointer to a GPXdoc struct
 **/
char* GPXtoJSON(const GPXdoc* gpx) {
    char* json = malloc(sizeof(char));
    strcpy(json, "");
    int size = 0;

    if (gpx == NULL)
    {
        size = strlen(json) + strlen("{}");
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "{}");
    }
    else
    {
        char* verStr = malloc(sizeof(char) * 30);
        sprintf(verStr, "{\"version\":%.1f,", gpx->version);
        size = strlen(json) + strlen(verStr) + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, verStr);
        free(verStr);

        char* ctrStr = gpx->creator;
        size = strlen(json) + strlen("\"creator\":\"") + strlen(ctrStr) + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "\"creator\":\"");
        strcat(json, ctrStr);

        int numWpt = getNumWaypoints(gpx);
        int numRte = getNumRoutes(gpx);
        int numTrk = getNumTracks(gpx);

        char* numStr = malloc(sizeof(char) * 60);
        sprintf(numStr, "\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", numWpt, numRte, numTrk);
        size = strlen(json) + strlen(numStr) + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, numStr);
        free(numStr);
    }

    return json;
}




/** Function to adding an Waypont struct to an existing Route struct
 *@pre arguments are not NULL
 *@post The new waypoint has been added to the Route's waypoint list
 *@return N/A
 *@param rt - a Route struct
 *@param pr - a Waypoint struct
 **/
void addWaypoint(Route* rt, Waypoint* pt) {
    if (rt != NULL && pt != NULL)
    {
        insertBack(rt->waypoints, pt);
    }
}

/** Function to adding an Route struct to an existing GPXdoc struct
 *@pre arguments are not NULL
 *@post The new route has been added to the GPXdoc's routes list
 *@return N/A
 *@param doc - a GPXdoc struct
 *@param rt - a Route struct
 **/
void addRoute(GPXdoc* doc, Route* rt) {
    if (doc != NULL && rt != NULL)
    {
        insertBack(doc->routes, rt);
    }
}

/** Function to converting a JSON string into an GPXdoc struct
 *@pre JSON string is not NULL
 *@post String has not been modified in any way
 *@return A newly allocated and initialized GPXdoc struct
 *@param str - a pointer to a string
 **/
GPXdoc* JSONtoGPX(const char* gpxString) {
    if (gpxString == NULL)
    {
        return NULL;
    }

    GPXdoc* tmpDoc = malloc(sizeof(GPXdoc));
    tmpDoc->version = 0.0;
    tmpDoc->creator = malloc(2 * sizeof(char*));
    strcpy(tmpDoc->creator, "");
    tmpDoc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    tmpDoc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    tmpDoc->tracks = initializeList(&trackToString, &deleteTrack, compareTracks);

    int i, j, ctr;
    char newString[10][10];

    j = 0; ctr = 0;
    for (i = 0; i <= (strlen(gpxString)); i++)
    {
        if (gpxString[i] == ',' || gpxString[i] == ':' || gpxString[i] == '\0')
        {
            newString[ctr][j] = '\0';
            ctr++;  //for next word
            j = 0;    //for next word, init index to 0
        }
        else if (gpxString[i] == '{' || gpxString[i] == '}' || gpxString[i] == '"')
        {
            //do nothing
        }
        else
        {
            newString[ctr][j] = gpxString[i];
            j++;
        }
    }

    for (i = 0; i <= 10; i++) {
        if (strcmp(newString[i], (char*)"version") == 0)
        {
            char* valueData = (char*)(newString[i+1]);
            double data = strtod(valueData, NULL);
            ((tmpDoc)->version) = data;
        }
        if (strcmp(newString[i], (char*)"creator") == 0)
        {
            char* creatorData = (char*)(newString[i + 1]);
            int size = strlen(creatorData) + 1;
            ((tmpDoc)->creator) = realloc((tmpDoc)->creator, sizeof(char) * size);
            strcpy(((tmpDoc)->creator), creatorData);
        }
    }

    char* ns = "http://www.topografix.com/GPX/1/1";
    strcpy(((tmpDoc)->namespace), ns);

    return tmpDoc;
}

/** Function to converting a JSON string into an Waypoint struct
 *@pre JSON string is not NULL
 *@post String has not been modified in any way
 *@return A newly allocated and initialized Waypoint struct
 *@param str - a pointer to a string
 **/
Waypoint* JSONtoWaypoint(const char* gpxString) {
    if (gpxString == NULL)
    {
        return NULL;
    }

    Waypoint* waypoint = malloc(sizeof(Waypoint));
    waypoint->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    waypoint->latitude = 0.0;
    waypoint->longitude = 0.0;
    waypoint->name = malloc(2 * sizeof(char*));
    strcpy(waypoint->name, "");

    int i, j, ctr;
    char newString[10][10];

    j = 0; ctr = 0;
    for (i = 0; i <= (strlen(gpxString)); i++)
    {
        if (gpxString[i] == ',' || gpxString[i] == ':' || gpxString[i] == '\0')
        {
            newString[ctr][j] = '\0';
            ctr++;  //for next word
            j = 0;    //for next word, init index to 0
        }
        else if (gpxString[i] == '{' || gpxString[i] == '}' || gpxString[i] == '"')
        {
            //do nothing
        }
        else
        {
            newString[ctr][j] = gpxString[i];
            j++;
        }
    }

    for (i = 0; i <= 10; i++) {
        if (strcmp(newString[i], (char*)"lat") == 0)
        {
            char* valueData = (char*)(newString[i + 1]);
            double data = strtod(valueData, NULL);
            (waypoint->latitude) = data;
        }
        if (strcmp(newString[i], (char*)"lon") == 0)
        {
            char* valueData = (char*)(newString[i + 1]);
            double data = strtod(valueData, NULL);
            (waypoint->longitude) = data;
        }
    }

    return waypoint;
}

/** Function to converting a JSON string into an Route struct
 *@pre JSON string is not NULL
 *@post String has not been modified in any way
 *@return A newly allocated and initialized Route struct
 *@param str - a pointer to a string
 **/
Route* JSONtoRoute(const char* gpxString) {
    if (gpxString == NULL)
    {
        return NULL;
    }

    Route* route = malloc(sizeof(Route));
    route->name = malloc(2 * sizeof(char*));
    strcpy(route->name, "");
    route->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    route->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);

    int i, j, ctr;
    char newString[10][10];

    j = 0; ctr = 0;
    for (i = 0; i <= (strlen(gpxString)); i++)
    {
        if (gpxString[i] == ',' || gpxString[i] == ':' || gpxString[i] == '\0')
        {
            newString[ctr][j] = '\0';
            ctr++;  //for next word
            j = 0;    //for next word, init index to 0
        }
        else if (gpxString[i] == '{' || gpxString[i] == '}' || gpxString[i] == '"')
        {
            //do nothing
        }
        else
        {
            newString[ctr][j] = gpxString[i];
            j++;
        }
    }

    for (i = 0; i <= 10; i++) {
        if (strcmp(newString[i], (char*)"name") == 0)
        {
            char* creatorData = (char*)(newString[i+1]);
            int size = strlen(creatorData) + 2;
            (route->name) = realloc(route->name, sizeof(char) * size);
            strcpy((route->name), creatorData);
        }
    }

    return route;
}


/* ******************************* List helper functions *************************** */
/*GPXData reference
typedef struct {
    char 	name[256];
    char	value[];
} GPXData;
*/

/** Function to delere all GPX data
 *@pre data object is not NULL
 *@param obj - a pointer to a data object
 **/
void deleteGpxData(void* data) {
    GPXData* tmpGPX = NULL;

    if (data != NULL) {
        tmpGPX = (GPXData*)data;
    }

    if (tmpGPX != NULL) {
        free(tmpGPX);
    }
}

/** Function to converting a data object to a string
 *@pre data object is not NULL
 *@post data has not been modified in any way
 *@return A newly created string
 *@param obj - a pointer to a data object
 **/
char* gpxDataToString(void* data) {
    GPXData* tmpGPX = (GPXData*)data;
    char* stringToReturn = malloc(sizeof(char));
    strcpy(stringToReturn, "");

    char* nameStr = tmpGPX->name;
    char* valueStr = tmpGPX->value;
    int size = strlen(stringToReturn) + strlen(": ") + strlen(nameStr) + strlen(valueStr) +  2;
    stringToReturn = realloc(stringToReturn, (sizeof(char) * size));
    strcat(stringToReturn, nameStr);
    strcat(stringToReturn, ": ");
    strcat(stringToReturn, valueStr);
    strcat(stringToReturn, "\n");

    return stringToReturn;
}

/** Function to compare two GPX data objects
 *@pre Both Objects are not NULL
 *@post both objects have not been modified in any way
 *@return An integer based off the results
 *@param obj- two pointers to objects each
 **/
int compareGpxData(const void* first, const void* second) {
    GPXData* tmpGPX1;
    GPXData* tmpGPX2;

    if (first == NULL || second == NULL) {
        return 0;
    }

    tmpGPX1 = (GPXData*)first;
    tmpGPX2 = (GPXData*)second;

    return strcmp((char*)tmpGPX1->name, (char*)tmpGPX2->name);
}

/*Waypoint reference
typedef struct {
    char* name;
    double longitude;
    double latitude;
    List* otherData;
} Waypoint;
*/

/** Function to delere all waypoint data
 *@pre data object is not NULL
 *@param obj - a pointer to a data object
 **/
void deleteWaypoint(void* data) {
    Waypoint* tmpWpt = NULL;

    if (data != NULL) {
        tmpWpt = (Waypoint*)data;
    }

    if (tmpWpt != NULL) {
        if (tmpWpt->name != NULL) {
            free(tmpWpt->name);
        }
        if (tmpWpt->latitude != 0.0) {
            tmpWpt->latitude = 0.0;
        }
        if (tmpWpt->longitude != 0.0) {
            tmpWpt->longitude = 0.0;
        }
        if (tmpWpt->otherData != NULL) {
            freeList(tmpWpt->otherData);
        }
    }
    free(tmpWpt);
}

/** Function to converting a waypint object to a string
 *@pre data object is not NULL
 *@post data has not been modified in any way
 *@return A newly created string
 *@param obj - a pointer to a data object
 **/
char* waypointToString(void* data) {
    Waypoint* tmpWpt = (Waypoint*)data;
    char* stringToReturn = malloc(sizeof(char));
    strcpy(stringToReturn, "");
    int size = 0;

    if (strcmp(tmpWpt->name, "") != 0) {
        char* nameStr = tmpWpt->name;
        size = strlen(stringToReturn) + strlen("Name: ") + strlen(nameStr) + 2;
        stringToReturn = realloc(stringToReturn, (sizeof(char) * size));
        strcat(stringToReturn, "Name: ");
        strcat(stringToReturn, nameStr);
        strcat(stringToReturn, "\n");
    }
    if (tmpWpt->latitude != 0.0) {
        char* latStr = malloc(sizeof(char) * 30);
        sprintf(latStr, "latitude: %f", tmpWpt->latitude);
        size = strlen(stringToReturn) + strlen(latStr) + 2;
        stringToReturn = realloc(stringToReturn, (sizeof(char) * size));
        strcat(stringToReturn, latStr);
        strcat(stringToReturn, "\n");
        free(latStr);
    }
    if (tmpWpt->longitude != 0.0) {
        char* lonStr = malloc(sizeof(char) * 30);
        sprintf(lonStr, "longitude: %f", tmpWpt->longitude);
        size = strlen(stringToReturn) + strlen(lonStr) + 2;
        stringToReturn = realloc(stringToReturn, (sizeof(char) * size));
        strcat(stringToReturn, lonStr);
        strcat(stringToReturn, "\n");
        free(lonStr);
    }
    if (tmpWpt->otherData != NULL) {
        char* otherStr = toString(tmpWpt->otherData);
        size = strlen(stringToReturn) + strlen("Other Data: ") + strlen(otherStr) + 2;
        stringToReturn = realloc(stringToReturn, (sizeof(char) * size));
        strcat(stringToReturn, "Other Data: ");
        strcat(stringToReturn, otherStr);
        strcat(stringToReturn, "\n");
        free(otherStr);
    }
    return stringToReturn;
}

/** Function to compare two waypoint data objects
 *@pre Both Objects are not NULL
 *@post both objects have not been modified in any way
 *@return An integer based off the results
 *@param obj- two pointers to objects each
 **/
int compareWaypoints(const void* first, const void* second) {
    Waypoint* tmpWPT1;
    Waypoint* tmpWPT2;

    if (first == NULL || second == NULL) {
        return 0;
    }

    tmpWPT1 = (Waypoint*)first;
    tmpWPT2 = (Waypoint*)second;

    return strcmp((char*)tmpWPT1->name, (char*)tmpWPT2->name);
}

/*Route reference
typedef struct {
    char* name;
    List* waypoints;
    List* otherData;
} Route;
*/

/** Function to delere all route data
 *@pre data object is not NULL
 *@param obj - a pointer to a data object
 **/
void deleteRoute(void* data) {
    Route* tmpRte = NULL;

    if (data != NULL) {
        tmpRte = (Route*)data;
    }

    if (tmpRte != NULL) {
        if (tmpRte->name != NULL) {
            free(tmpRte->name);
        }
        if (tmpRte->waypoints != NULL) {
            freeList(tmpRte->waypoints);
        }
        if (tmpRte->otherData != NULL) {
            freeList(tmpRte->otherData);
        }
    }
    free(tmpRte);
}

/** Function to converting a route object to a string
 *@pre data object is not NULL
 *@post data has not been modified in any way
 *@return A newly created string
 *@param obj - a pointer to a data object
 **/
char* routeToString(void* data) {
    Route* tmpRte = (Route*)data;
    char* stringToReturn = malloc(sizeof(char));
    strcpy(stringToReturn, "");
    int size = 0;

    if (strcmp(tmpRte->name, "") != 0) {
        char* nameStr = tmpRte->name;
        size = strlen(stringToReturn) + strlen("Name: ") + strlen(nameStr) + 2;
        stringToReturn = realloc(stringToReturn, (sizeof(char) * size));
        strcat(stringToReturn, "Name: ");
        strcat(stringToReturn, nameStr);
        strcat(stringToReturn, "\n");
    }
    if (tmpRte->otherData != NULL) {
        char* otherStr = toString(tmpRte->otherData);
        size = strlen(stringToReturn) + strlen("Other Data: ") + strlen(otherStr) + 2;
        stringToReturn = realloc(stringToReturn, (sizeof(char) * size));
        strcat(stringToReturn, "Other Data: ");
        strcat(stringToReturn, otherStr);
        strcat(stringToReturn, "\n");
        free(otherStr);
    }
    if (tmpRte->waypoints != NULL) {
        char* wptStr = toString(tmpRte->waypoints);
        size = strlen(stringToReturn) + strlen("Waypoints: ") + strlen(wptStr) + 2;
        stringToReturn = realloc(stringToReturn, (sizeof(char) * size));
        strcat(stringToReturn, "Waypoints: ");
        strcat(stringToReturn, wptStr);
        strcat(stringToReturn, "\n");
        free(wptStr);
    }
    return stringToReturn;
}

/** Function to compare two route data objects
 *@pre Both Objects are not NULL
 *@post both objects have not been modified in any way
 *@return An integer based off the results
 *@param obj- two pointers to objects each
 **/
int compareRoutes(const void* first, const void* second) {
    Route* tmpRTE1;
    Route* tmpRTE2;

    if (first == NULL || second == NULL) {
        return 0;
    }

    tmpRTE1 = (Route*)first;
    tmpRTE2 = (Route*)second;

    return strcmp((char*)tmpRTE1->name, (char*)tmpRTE2->name);
}

/*Track Segment refernece
typedef struct {
    List* waypoints;
} TrackSegment;
*/

/** Function to delere all track segments data
 *@pre data object is not NULL
 *@param obj - a pointer to a data object
 **/
void deleteTrackSegment(void* data) {
    TrackSegment* tmpTrSeg = NULL;

    if (data != NULL) {
        tmpTrSeg = (TrackSegment*)data;
    }

    if (tmpTrSeg != NULL) {
        if (tmpTrSeg->waypoints != NULL) {
            freeList(tmpTrSeg->waypoints);
        }
    }
    free(tmpTrSeg);
}

/** Function to converting a track segments object to a string
 *@pre data object is not NULL
 *@post data has not been modified in any way
 *@return A newly created string
 *@param obj - a pointer to a data object
 **/
char* trackSegmentToString(void* data) {
    TrackSegment* tmpTrSeg= (TrackSegment*)data;
    char* stringToReturn = malloc(sizeof(char));
    strcpy(stringToReturn, "");
    int size = 0;

    if (tmpTrSeg->waypoints != NULL) {
        char* wptStr = toString(tmpTrSeg->waypoints);
        size = strlen(stringToReturn) + strlen("Waypoints: ") + strlen(wptStr) + 2;
        stringToReturn = realloc(stringToReturn, (sizeof(char) * size));
        strcat(stringToReturn, "Waypoints: ");
        strcat(stringToReturn, wptStr);
        strcat(stringToReturn, "\n");
        free(wptStr);
    }

    return stringToReturn;
}

/** Function to compare two track segments data objects
 *@pre Both Objects are not NULL
 *@post both objects have not been modified in any way
 *@return An integer based off the results
 *@param obj- two pointers to objects each
 **/
int compareTrackSegments(const void* first, const void* second) {
    TrackSegment* tmpTS1;
    TrackSegment* tmpTS2;

    if (first == NULL || second == NULL) {
        return 0;
    }

    tmpTS1 = (TrackSegment*)first;
    tmpTS2 = (TrackSegment*)second;

    int length1 = tmpTS1->waypoints->length;
    int length2 = tmpTS2->waypoints->length;

    return (length1 - length2);
}

/*Track reference
typedef struct {
    char* name;
    List* segments;
    List* otherData;
} Track;
*/

/** Function to delere all track data
 *@pre data object is not NULL
 *@param obj - a pointer to a data object
 **/
void deleteTrack(void* data) {
    Track* tmpTrk = NULL;

    if (data != NULL) {
        tmpTrk = (Track*)data;
    }

    if (tmpTrk != NULL) {
        if (tmpTrk->name != NULL) {
            free(tmpTrk->name);
        }
        if (tmpTrk->segments != NULL) {
            freeList(tmpTrk->segments);
        }
        if (tmpTrk->otherData != NULL) {
            freeList(tmpTrk->otherData);
        }
    }
    free(tmpTrk);
}

/** Function to converting a track object to a string
 *@pre data object is not NULL
 *@post data has not been modified in any way
 *@return A newly created string
 *@param obj - a pointer to a data object
 **/
char* trackToString(void* data) {
    Track* tmpTrk = (Track*)data;
    char* stringToReturn = malloc(sizeof(char));
    strcpy(stringToReturn, "");
    int size = 0;

    if (strcmp(tmpTrk->name, "") != 0) {
        char* nameStr = tmpTrk->name;
        size = strlen(stringToReturn) + strlen("Name: ") + strlen(nameStr) + 2;
        stringToReturn = realloc(stringToReturn, (sizeof(char) * size));
        strcat(stringToReturn, "Name: ");
        strcat(stringToReturn, nameStr);
        strcat(stringToReturn, "\n");
    }
    if (tmpTrk->otherData != NULL) {
        char* otherStr = toString(tmpTrk->otherData);
        size = strlen(stringToReturn) + strlen("Other Data: ") + strlen(otherStr) + 2;
        stringToReturn = realloc(stringToReturn, (sizeof(char) * size));
        strcat(stringToReturn, "Other Data: ");
        strcat(stringToReturn, otherStr);
        strcat(stringToReturn, "\n");
        free(otherStr);
    }
    if (tmpTrk->segments != NULL) {
        char* segStr = toString(tmpTrk->segments);
        size = strlen(stringToReturn) + strlen("Segments: ") + strlen(segStr) + 2;
        stringToReturn = realloc(stringToReturn, (sizeof(char) * size));
        strcat(stringToReturn, "Segments: ");
        strcat(stringToReturn, segStr);
        strcat(stringToReturn, "\n");
        free(segStr);
    }

    return stringToReturn;
}

/** Function to compare two track data objects
 *@pre Both Objects are not NULL
 *@post both objects have not been modified in any way
 *@return An integer based off the results
 *@param obj- two pointers to objects each
 **/
int compareTracks(const void* first, const void* second) {
    Track* tmpTRK1;
    Track* tmpTRK2;

    if (first == NULL || second == NULL) {
        return 0;
    }

    tmpTRK1 = (Track*)first;
    tmpTRK2 = (Track*)second;

    return strcmp((char*)tmpTRK1->name, (char*)tmpTRK2->name);
}

/** Function to convert an XML file to JSON
 *@pre FileName is not NULL
 *@post File has not been modified in any way
 *@return A JSON string with data from the file
 *@param str- a string literal of the parsed JSON data
 **/
char* FiletoJSON(char* fileName) {
    GPXdoc* tmpDoc = malloc(sizeof(GPXdoc));
    char* json = malloc(sizeof(char));
    strcpy(json, "");
    int size = 0;

    tmpDoc = createGPXdoc(fileName);

    char* jStr = GPXtoJSON(tmpDoc);
    size = strlen(json) + strlen(jStr) +2;
    json = realloc(json, (sizeof(char) * size));
    strcat(json, jStr);
    free(jStr);

    deleteGPXdoc(tmpDoc);

    return json;
}

/** Function to convert an GPX file to JSON
 *@pre FileName is not NULL
 *@post File has not been modified in any way
 *@return A JSON string with data from the file
 *@param str- a string literal of the parsed JSON data
 **/
char* GPXViewtoJSON(char* fileName) {
    GPXdoc* tmpDoc = malloc(sizeof(GPXdoc));
    char* json = malloc(sizeof(char));
    strcpy(json, "");
    int size = 0;

    tmpDoc = createGPXdoc(fileName);

    char* rStr = routeListToJSON(tmpDoc->routes);
    char* tStr = trackListToJSON(tmpDoc->tracks);
    char* rdStr = routeDataToJSON(tmpDoc->routes);
    char* tdStr = trackDataToJSON(tmpDoc->tracks);

    size = strlen(json) + strlen(rStr) + strlen("--") + strlen(tStr) + strlen("--") + strlen(rdStr) + strlen("--") + strlen(tdStr) + 2;
    json = realloc(json, (sizeof(char) * size));
    strcat(json, rStr);
    strcat(json, "--");
    strcat(json, tStr);
    strcat(json, "--");
    strcat(json, rdStr);
    strcat(json, "--");
    strcat(json, tdStr);
    free(rStr);
    free(tStr);
    free(rdStr);
    free(tdStr);

    deleteGPXdoc(tmpDoc);

    return json;
}

/** Function to convert track data to JSON
 *@pre Linked list is not NULL
 *@post Linked list has not been modified in any way
 *@return A JSON string with data from the file
 *@param str- a string literal of the parsed JSON data
 **/
char* trackDataToJSON(const List* list) {
    char* json = malloc(sizeof(char));
    strcpy(json, "");
    int size = 0;
    bool notFirst = false;

    if (list == NULL)
    {
        size = strlen(json) + strlen("[]");
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "[]");
    }
    else
    {
        size = strlen(json) + strlen("[") + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "[");

        ListIterator iter;

        iter.current = list->head;
        void* trk;

        while ((trk = nextElement(&iter)) != NULL) {
            Track* tmpTrk = (Track*)trk;

            char* jsonTrkStr = otherDataToJSON(tmpTrk->otherData);
            size = strlen(json) + strlen(jsonTrkStr) + 2;
            json = realloc(json, (sizeof(char) * size));
            if (notFirst)
            {
                size = strlen(json) + strlen(",") + 2;
                json = realloc(json, (sizeof(char) * size));
                strcat(json, ",");
            }
            strcat(json, jsonTrkStr);
            notFirst = true;
        }

        size = strlen(json) + strlen("]") + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "]");
    }

    return json;
}

/** Function to convert route data to JSON
 *@pre Linked list is not NULL
 *@post Linked list has not been modified in any way
 *@return A JSON string with data from the file
 *@param str- a string literal of the parsed JSON data
 **/
char* routeDataToJSON(const List* list) {
    char* json = malloc(sizeof(char));
    strcpy(json, "");
    int size = 0;
    bool notFirst = false;

    if (list == NULL)
    {
        size = strlen(json) + strlen("[]");
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "[]");
    }
    else
    {
        size = strlen(json) + strlen("[") + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "[");

        ListIterator iter;

        iter.current = list->head;
        void* rte;

        while ((rte = nextElement(&iter)) != NULL) {
            Route* tmpRte = (Route*)rte;

            char* jsonRteStr = otherDataToJSON(tmpRte->otherData);
            size = strlen(json) + strlen(jsonRteStr) + 2;
            json = realloc(json, (sizeof(char) * size));
            if (notFirst)
            {
                strcat(json, ",");
            }
            strcat(json, jsonRteStr);
            notFirst = true;
        }

        size = strlen(json) + strlen("]") + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "]");
    }

    return json;
}

/** Function to convert Other waypoint data to JSON
 *@pre Linked list is not NULL
 *@post Linked list has not been modified in any way
 *@return A JSON string with data from the file
 *@param str- a string literal of the parsed JSON data
 **/
char* otherDataToJSON(const List* list) {
    char* json = malloc(sizeof(char));
    strcpy(json, "");
    int size = 0;
    bool notFirst = false;

    if (list == NULL)
    {
        size = strlen(json) + strlen("{}");
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "{}");
    }
    else
    {
        size = strlen(json) + strlen("{") + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "{");

        ListIterator iter;

        iter.current = list->head;
        void* data;

        while ((data = nextElement(&iter)) != NULL) {
            GPXData* tmpData = (GPXData*)data;

            char* nameStr = tmpData->name;
            char* valStr = tmpData->value;
            size = strlen(json) + strlen("\"name\":\"") + strlen(nameStr) + strlen("\",\"value\":\"") + strlen(valStr) + strlen("\"") + 2;
            json = realloc(json, (sizeof(char) * size));
            
            if (notFirst)
            {
                strcat(json, ",");
            }

            strcat(json, "\"name\":\"");
            strcat(json, nameStr);
            strcat(json, "\",\"value\":\"");
            strcat(json, valStr);
            strcat(json, "\"");

            notFirst = true;
        }

        size = strlen(json) + strlen("}") + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "}");
    }

    return json;
}

/** Function to find a path between a source and destination
 *@pre filename is not NULL
 *@return A String literal of the JSON data. 
 *@param 
	str- a string literal of the filename
	float- the source lat and long
	float- the destination lat and long
	float- the delta value
 **/
char* findPathToJSON(char* fileName, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {
    GPXdoc* tmpDoc = malloc(sizeof(GPXdoc));
    char* json = malloc(sizeof(char));
    strcpy(json, "");
    int size = 0;
    bool notFirst = false;
    bool data = false;

    tmpDoc = createGPXdoc(fileName);
    if (tmpDoc->routes != NULL)
    {
        List* list = getRoutesBetween(tmpDoc, sourceLat, sourceLong, destLat, destLong, delta);
        if (list != NULL) {
            char* rStr = routeListToJSON(list);

            if (notFirst)
            {
                size = strlen(json) + strlen("--") + 2;
                json = realloc(json, (sizeof(char) * size));
                strcat(json, "--");
            }

            size = strlen(json) + strlen(rStr) + 2;
            json = realloc(json, (sizeof(char) * size));
            strcat(json, rStr);
            free(rStr);

            notFirst = true;
            data = true;
        }
    }
    if (tmpDoc->tracks != NULL)
    {
        List* list = getTracksBetween(tmpDoc, sourceLat, sourceLong, destLat, destLong, delta);
        if (list != NULL) {
            char* tStr = trackListToJSON(list);

            if (notFirst)
            {
                size = strlen(json) + strlen("--") + 2;
                json = realloc(json, (sizeof(char) * size));
                strcat(json, "--");
            }

            size = strlen(json) + strlen(tStr) + 2;
            json = realloc(json, (sizeof(char) * size));
            strcat(json, tStr);
            free(tStr);

            notFirst = true;
            data = true;
        }
    }
    if (!data) {
        size = strlen(json) + strlen("[]") + 2;
        json = realloc(json, (sizeof(char) * size));
        strcat(json, "[]");
    }

    return json;
}

/** Function to add a new GPX and check if it is correct. 
 *@pre filename is not NULL
 *@return A Boolean if successful or not
 *@param 
	str- a string literal of the filename
	str- a string literal of the parsed JSON data
	str- a string literal of the parsed JSON SchemaFile
 **/
bool createNewGPX(char* fileName, char* JSONString, char* gpxSchemaFile) {
    GPXdoc* tmpGPX = malloc(sizeof(GPXdoc));
    bool result = false;
    tmpGPX = JSONtoGPX(JSONString);
    if (validateGPXDoc(tmpGPX, gpxSchemaFile)) {
        writeGPXdoc(tmpGPX, fileName);
        result = true;
    }
    return result;
}

/** Function to add a new route to a file
 *@pre filename is not NULL
 *@return A Boolean if successful or not
 *@param 
	str- a string literal of the filename
	str- a string literal of the parsed JSON route data
	str- a string literal of the parsed JSON waypoint data
	int- number of waypoints
 **/
bool addNewRoute(char* fileName, char* routeJSON, char* wptJSON, int numWPT) {
    GPXdoc* tmpDoc = malloc(sizeof(GPXdoc));
    Route* route = malloc(sizeof(Route));

    tmpDoc = createGPXdoc(fileName);
    route = JSONtoRoute(routeJSON);

    int i, j, ctr;
    char newString[numWPT][100];

    j = 0; ctr = 0;
    for (i = 0; i <= (strlen(wptJSON)); i++)
    {
        if (wptJSON[i] == ']' || wptJSON[i] == '\0')
        {
            newString[ctr][j] = '\0';
            ctr++;  //for next word
            j = 0;    //for next word, init index to 0
        }
        else if (wptJSON[i] == '[')
        {
            //do nothing
        }
        else
        {
            newString[ctr][j] = wptJSON[i];
            j++;
        }
    }

    for (i = 0; i < numWPT; i++) {
        Waypoint* waypoint = malloc(sizeof(Waypoint));
        waypoint = JSONtoWaypoint(newString[i]);
        insertBack(route->waypoints, waypoint);
    }

    insertBack(tmpDoc->routes, route);

    writeGPXdoc(tmpDoc, fileName);
    deleteGPXdoc(tmpDoc);
    bool result = true;

    return result;
}