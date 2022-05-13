'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app = express();
const path = require("path");
const fileUpload = require('express-fileupload');

// File I/O setup
app.use(fileUpload());
app.use(express.static(path.join(__dirname + '/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/', function (req, res) {
    res.sendFile(path.join(__dirname + '/public/index.html'));
});

// Send Style, do not change
app.get('/style.css', function (req, res) {
    //Feel free to change the contents of style.css to prettify your Web app
    res.sendFile(path.join(__dirname + '/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js', function (req, res) {
    fs.readFile(path.join(__dirname + '/public/index.js'), 'utf8', function (err, contents) {
        const minimizedContents = JavaScriptObfuscator.obfuscate(contents, { compact: true, controlFlowFlattening: true });
        res.contentType('application/javascript');
        res.send(minimizedContents._obfuscatedCode);
    });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function (req, res) {
    if (!req.files) {
        return res.status(400).send('No files were uploaded.');
    }

    let uploadFile = req.files.uploadFile;

    // Use the mv() method to place the file somewhere on your server
    uploadFile.mv('uploads/' + uploadFile.name, function (err) {
        if (err) {
            return res.status(500).send(err);
        }

        res.redirect('/');
    });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function (req, res) {
    fs.stat('uploads/' + req.params.name, function (err, stat) {
        if (err == null) {
            res.sendFile(path.join(__dirname + '/uploads/' + req.params.name));
        } else {
            console.log('Error in file downloading route: ' + err);
            res.send('');
        }
    });
}); 

// Library Directory
let gpxLib = ffi.Library('./parser/bin/libgpxparser.so', {
    'FiletoJSON': ['string', ['string']],
    'GPXViewtoJSON': ['string', ['string']],
    'findPathToJSON': ['string', ['string', 'float', 'float', 'float', 'float', 'float']],
    'createNewGPX': ['bool', ['string', 'string', 'string']],
    'addNewRoute': ['bool', ['string', 'string', 'string', 'int']]
});

var connection;

// Helper funtions
// Load GPX file formats to JSON
//Inputs String: Filename
//Returns String: JSON
function getGPXdoc(filename) {
    let fname = "./uploads/" + filename;
    let jsonStr = gpxLib.FiletoJSON(fname);
    return jsonStr;
}

// Load WPT file formats to JSON
//Inputs String: Filename
//Returns String: JSON
function getWPTJSON(filename) {
    let fname = "./uploads/" + filename;
    let jStr = gpxLib.GPXViewtoJSON(fname);
    return jStr;
}

// Request FileLog from upoaded files
//returns number of files, filenames and the gpx files themselves
app.get('/getFileLog', function (req, res) {
    fs.readdir('./uploads', (err, files) => {
        var GPX = [];
        var fNames = [];
        let size = 0;
        let num = files.length;
        for (let i = 0; i < num; i++) {
            if (files[i].endsWith(".gpx")) {
                var json = getGPXdoc(files[i]);
                if (json !== null) {
                    if (json !== '{}') {
                        size++;
                        GPX.push(json);
                        fNames.push(files[i]);
                    }
                }
            }
        }
        res.send({
            numFiles: size,
            fileNames: fNames,
            files: GPX
        });
    });
});

// Request GPX data from upoaded files
//returns list of routes, tracks, and their data
app.get('/getGPXView', function (req, res) {
    var data = [];
    var routes = [];
    var tracks = [];
    var routeData = [];
    var trackData = [];
    let file = req.query.file;

    let alljson = getWPTJSON(file);
    data = alljson.split('--');
    routes = data[0];
    tracks = data[1];
    routeData = data[2];
    trackData = data[3];

    res.send({
        routesList: routes,
        tracksList: tracks,
        routeOtherData: routeData,
        trackOtherData: trackData
    });
});

// finds the best bath from start to destination points based on latitude and longitude.
//returns number of paths found and the data for those paths
app.get('/findPath', function (req, res) {
    fs.readdir('./uploads', (err, files) => {
        var pathData = [];
        let slat = req.query.slat;
        let slon = req.query.slon;
        let dlat = req.query.dlat;
        let dlon = req.query.dlon;
        let delta = req.query.delta;
        let size = 0;
        let num = files.length;
        for (let i = 0; i < num; i++) {
            if (files[i].endsWith(".gpx")) {
                let fname = "./uploads/" + files[i];
                let json = gpxLib.findPathToJSON(fname, slat, slon, dlat, dlon, delta);
                if (json !== null) {
                    let datap = json.split('--');
                    for (let j = 0; j < datap.length; j++) {
                        if (datap[j] !== '[]') {
                            size++;
                            pathData.push(datap[j]);
                        }
                    }
                }
            }
        }
        res.send({
            numPathFiles: size,
            foundPathData: pathData
        });
    });
});

// Create GPX file from data provided
//returns new GPX file
app.get('/create', function (req, res) {
    let version = req.query.version;
    let creator = req.query.creator;
    let file = './uploads/' + req.query.fName + '.gpx';
    let validationFile = "./uploads/gpx.xsd";
    var jsonData = '{"version":' + version + ',"creator":"' + creator + '"}';

    let result = gpxLib.createNewGPX(file, jsonData, validationFile);

    res.send({
        Result: result
    });
});

// Add a New Route to the path.
//Returns true or false based on success. 
app.get('/addRoute', function (req, res) {
    let wpt = req.query.wptJSON;
    let route = req.query.routeJSON;
    let file = './uploads/' + req.query.fileName;
    let num = req.query.wptNum;

    let wptFull = '';

    for (let w = 0; w < wpt.length; w++) {
        wptFull += '[' + wpt[w] + ']';
    }

    let result = gpxLib.addNewRoute(file, route, wptFull, num);

    res.send({
        Result: result
    });
});

// Connection to the SQL database. (used my schools server at the time, can easilly chnage to another SQL database)
//returns string with server name or error code. 
app.get('/connect', function (req, res) {
    let dbInfo = {
        host: 'dursley.socs.uoguelph.ca',
        user: req.query.username,
        password: req.query.password,
        database: req.query.dbname
    }

    let result;
    const mysql = require('mysql2/promise');

    try {
        connection = mysql.createConnection(dbInfo)
        result = 'dursley.socs.uoguelph.ca';
    } catch (e) {
        result = "Connection error: " + e;
    }

    res.send({
        Result: result
    });
});

// Clears the data from the SQL database
// returns String: error or success.  
app.get('/clearData', function (req, res) {
    let result = 'Success!';
    connection.query('DELETE FROM POINT; DELETE FROM ROUTE; DELETE FROM FILE;', function (error, results) {
        if (error) {
            result = 'Error: Unable to clear table!';
        }
        res.send({
            Result: result
        });
    });
});

// Sames the uploaded documents to the SQL database
//return 200 success if completed
app.get('/storeFiles', function (req, res) {
    fs.readdir('./uploads', (err, files) => {
        let prior = 0;
        let num = files.length;
        for (let i = 0; i < num; i++) {
            if (files[i].endsWith(".gpx")) {
                var json = getGPXdoc(files[i]);
                if (json !== null) {
                    let dataToSend = JSON.parse(json);
                    insertFile(file, dataToSend, prior);
                    prior += json.numEvents;
                }
            }
        }
    });
    res.send('Success!');
});

// insert a file into the Database givin the filename and data.
//Input: string: filename GPX file: data
function insertFile(filename, data, eventsPrior) {
    if (connection) {
        connection.query('SELECT * FROM FILE WHERE file_Name = "' + filename + '"', (error, result) => {
            if (error) {
                console.log('Error occured while trying to check if ' + filename + ' exists!');
                return;
            } else {
                if (result.length < 0) {
                    let rec = 'INSERT INTO FILE (file_Name, ver, creator) VALUES (\'' + filename + '\', ' + data.version + ', \'' + data.creator + '\');';
                    connection.query(rec, (error, result) => {
                        if (error) {
                            console.log(error);
                            return;
                        } else {
                            let data2 = [];

                            let alljson = getWPTJSON(filename);
                            data2 = alljson.split('--');
                            routes = data2[0];
                            for (let i = 0; i < data.numRoutes; i++) {

                                let numPoints = routes[i].numPoints;

                                connection.query('INSERT INTO ROUTE SET route_name = ' + routes[i].name + ', route_len = ' + routes[i].len + ', gpx_id = (SELECT gpx_id FROM FILE WHERE file_Name = "' + filename + '");', (error2, result2) => {
                                    if (error2) {
                                        console.log(error2);
                                        return;
                                    }
                                });
                            }
                        }
                    });
                }
            }
        });
    }
}

//SQL Querry options  
app.get('/query', function (req, res) {
    let num = req.query.num;
    let result;
    let table = {
        '1': 'SELECT * FROM ROUTE ORDER BY ' + req.query.order + ';',
        '2': 'SELECT * FROM ROUTE WHERE gpx_id = (SELECT gpx_id FROM FILE WHERE file_Name = "' + req.query.filename + '" ) ORDER BY ' + req.query.order + ';',
        '3': 'SELECT * FROM POINT WHERE route_id = (SELECT route_id FROM ROUTE WHERE route_name = "' + req.query.routename + '" ) ORDER BY point_index;',
        '4': 'SELECT * FROM POINT WHERE route_id = (SELECT route_id FROM ROUTE WHERE gpx_id = (SELECT gpx_id FROM FILE WHERE file_Name = "' + req.query.filename + '" )) ;',
        //'5': 
    }
    connection.query(table[new String(num)], (err, res) => {
        if (err) {
            console.log(err);
            result = 'There occured an error on the server.';
        }
        if (res) {
            result = res;
        }
        res.send({
            Result: result
        });
    });
});

// Get the current status and data within database
//return: 400 error code or 200 success with results. 
app.get('/dbstatus', function (req, res) {
    connection.query('SELECT (SELECT COUNT(*) FROM FILE) AS filecount, (SELECT COUNT(*) FROM ROUTE) AS routecount, (SELECT COUNT(*) FROM POINT) AS pointcount;', function (error, result) { //'SELECT COUNT(gpx_id) AS fc FROM File'
        if (error) {
            res.status(400).send('Could not retrieve db status');
        } else {
            res.send({ 'FILE': result[0]['filecount'], 'ROUTE': result[0]['routecount'], 'POINT': result[0]['pointcount'] });
        }
    });
});


//Sample endpoint
app.get('/endpoint1', function (req, res) {
    let retStr = req.query.stuff + " " + req.query.junk;
    res.send({
        stuff: retStr
    });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);