var docs = {};
var fileCount = 0;
var fileNames = {};
var routeOtherData = {};
var trackOtherData = {};
var obj = {};
var wptNum = 1;
var fileDropDown = '';
var connected = false;

// Put all onload AJAX calls here, and event listeners
$(document).ready(function () {
    updateFileLog();
    $('#options').hide();
    $('#querry').hide();
    // On page-load AJAX Example
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/endpoint1',   //The server endpoint we are connecting to
        data: {
            stuff: "Value 1",
            junk: "Value 2"
        },
        success: function (data) {
            /*  Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
            $('#blah').html("On page load, received string '" + data.stuff + "' from server");
            //We write the object to the console to show that the request was successful
            console.log(data);

        },
        fail: function (error) {
            // Non-200 return, do something with error
            $('#blah').html("On page load, received error from server");
            console.log(error);
        }
    });

    // Event listener form example , we can use this instead explicitly listening for events
    // No redirects if possible
    $('#someform').submit(function (e) {
        $('#blah').html("Form has data: " + $('#entryBox').val());
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            //Create an object for connecting to another waypoint
        });
    });

    $('#findPath').submit(function (e) {
        findPaths();
        e.preventDefault();
    });

    $('#createFile').submit(function (e) {
        createNewGPX();
        e.preventDefault();
    });

    $('#addRoute').submit(function (e) {
        addRTE();
        e.preventDefault();
    });

    $('#uploadForm').submit(updateFileLog);

    $('#fileDropDown').on('change', function(e) {
        fileDropDown = $(this).val();
        updateGPXVeiw();
        e.preventDefault();
    });

    $('#connectform').submit(function (e) {
        connect();
        e.preventDefault();
    });
});

// Given the user input, grad the data and call the server to find the paths between
//input: source and destination coordinates and a delta value. 
//return: data to display on webpage
function findPaths() {
    $('#findPathPanel').empty();
    $('#pathStatusMessage').empty();

    let srcLat = $('#srcLat').val();
    let srcLon = $('#srcLon').val();
    let destLat = $('#destLat').val();
    let destLon = $('#destLon').val();
    let delta = $('#delta').val();

    if (srcLat === undefined || srcLon === undefined || destLat === undefined || destLon === undefined || delta === undefined) {
        alert('Missing Data: Please enter a valid Lats, Lons and delta!');
    } else if (srcLat < -90.0 || srcLat > 90.0 || destLat < -90.0 || destLat > 90.0) {
        alert('Please enter a valid latitude!');
    } else if (srcLon < -180.0 || srcLon > 180.0 || destLon < -180.0 || destLon > 180.0) {
        alert('Please enter a valid longitude!');
    } else if (delta < 0) {
        alert('Please enter a valid delta!');
    } else {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/findPath',
            data: {
                slat: srcLat,
                slon: srcLon,
                dlat: destLat,
                dlon: destLon,
                delta: delta
            },
            success: function (res) {
                if (res['numPathFiles'] === 0) {
                    $('#pathStatusMessage').append('There are NO matching paths on the server.');
                } else {
                    $('#pathStatusMessage').append('There are <span>' + res['numPathFiles'] + '</span> matching paths on the server.');
                }

                pathObj = res['foundPathData'];

                //Append them
                for (let i = 0; i < res['numPathFiles']; i++) {
                    let data = JSON.parse(pathObj[i]);
                    var rowp = '<tr><td>' + data[i].name + '</td><td>' + data[i].numPoints + '</td><td>' + data[i].len + '</td><td>' + data[i].loop + '</td></tr>';
                    $('#findPathPanel').append(rowp);
                };
            },
            fail: function (error) {
                console.log(error);
            }
        });
    }
}

// If user wishes to view more data on a GPX value, then display the data if any is present. 
function showOtherData(element) {
    let elementNum = element.closest('tr').rowIndex;
    let num = elementNum - 1;
    var heading = "";
    var data = "";

    if (routeOtherData.length >= elementNum) {
        heading = 'Route' + elementNum + ' Other Data:\n';
        if (routeOtherData[num].name === undefined) {
            data = ' - No Other Data.';
        } else if (routeOtherData[num].name === 'desc') {
            data = ' - Description : ' + routeOtherData[num].value + '.';
        } else if (routeOtherData[num].name === 'ele') {
            data = ' - Elevation : ' + routeOtherData[num].value + '.';
        } else {
            data = ' - ' + routeOtherData[num].name + ' : ' + routeOtherData[num].value + '.';
        }
        
    } else {
        let newNum = elementNum - routeOtherData.length;
        let num2 = newNum - 1;
        heading = 'Track' + newNum + ' Other Data:\n';
        if (trackOtherData[num2].name === undefined) {
            data = ' - No Other Data.';
        } else if (trackOtherData[num2].name === 'desc') {
            data = ' - Description : ' + trackOtherData[num2].value + '.';
        } else if (trackOtherData[num2].name === 'ele') {
            data = ' - Elevation : ' + trackOtherData[num2].value + '.';
        } else {
            data = ' - ' + trackOtherData[num2].name + ' : ' + trackOtherData[num2].value + '.';
        }
    }
    alert(heading + data);
}

// change the name of a GPX component based off what the user types.
function changeName(element) {
    let newName = $('#compName').val();
    let rowNum = element.closest('tr').rowIndex;
    var myTable = document.getElementById('gpxTable');
    if (newName.length === 0 || newName === undefined) {
        alert('Invalid Name.');
    } else {
        myTable.rows[rowNum].cells[1].innerHTML = newName;
    }
}

//clear the filelog and checks the server to update what has changed.
function updateFileLog() {
    /* Clear */
    $('#statusMessage').empty();
    $('#fileLogPanel').empty();
    $('#fileDropDown').empty();
    $('#fileDropDown').append('<option value="" disabled selected>Choose a file</option>');

    $.ajax({
        type: "GET",
        url: "/getFileLog",
        dataType: "JSON",
        success: function (res) {

            if (res['numFiles'] === 0) {
                $('#statusMessage').append('There are NO valid files on the server.');
            } else {
                $('#statusMessage').append('There are <span>' + res['numFiles'] + '</span> valid file(s) on the server.');
            }

            fileCount = res['numFiles'];
            fileNames = res['fileNames'];
            obj = res['files'];

            for (let i = 0; i < fileCount; i++) {
                let data = JSON.parse(obj[i]);
                var row = '<tr><td><a href="' + fileNames[i] + '" download ">' + fileNames[i] + '</a></td><td>' + data.version + '</td><td>' + data.creator + '</td><td>' + data.numWaypoints + '</td><td>' + data.numRoutes + '</td><td>' + data.numTracks + '</td></tr>';
                $('#fileLogPanel').append(row);
                $('#fileDropDown').append('<option value="' + fileNames[i] + '">' + fileNames[i] + '</option>');
            };
        },
        fail: function (error) {
            console.log(error);
        }
    });
}

//given a filename, this will ask the server t create a new GPX value with the filename selected.
function createNewGPX() {
    let fname = $('#fileNameEntry').val();
    let ver = 1.1;
    let ctr = 'UofG';

    if (fname.length <= 0 || fname.length > 100 || fname.includes('.') || fname.includes(' ') || fname === undefined) {
        alert('Could not create GPX: Invalid File Name');
    } else {
        $.ajax({
            type: "GET",
            url: "/create",
            dataType: "JSON",
            data: {
                fName: fname,
                version: ver,
                creator: ctr
            },
            success: function (res) {
                updateFileLog()
            },
            fail: function (error) {
                console.log('Could not upload file' + error);
            }
        });
    }
}

// add new tracks or routes to a component based off user inputs. 
//chanks to see what the user has input and calls the server to update the component. 
function updateGPXVeiw() {
    $('#gpxStatusMessage').empty();
    $('#gpxViewPanel').empty();
    let sum = 0;

    $.ajax({
        type: "GET",
        url: "/getGPXView",
        dataType: "JSON",
        data: {
            file: fileDropDown
        },
        success: function (res) {
            let routes = res['routesList'];
            let tracks = res['tracksList'];
            let ROData = res['routeOtherData'];
            let TOData = res['trackOtherData'];
            routeOtherData = JSON.parse(ROData);
            trackOtherData = JSON.parse(TOData);

            if (routes !== '[]') {
                let dataR = JSON.parse(routes);
                sum += dataR.length;
                for (let r = 0; r < dataR.length; r++) {
                    let num = r + 1;
                    var rowr = '<tr><td style="cursor: pointer" onclick="showOtherData(this)">Route' + num + '</td><td style="cursor: pointer" onclick="changeName(this)">' + dataR[r].name + '</td><td>' + dataR[r].numPoints + '</td><td>' + dataR[r].len + '</td><td>' + dataR[r].loop + '</td></tr>';
                    $('#gpxViewPanel').append(rowr);
                }
            }

            if (tracks !== '[]') {
                let dataT = JSON.parse(tracks);
                sum += dataT.length;
                for (let t = 0; t < dataT.length; t++) {
                    let num = t + 1;
                    var rowt = '<tr><td style="cursor: pointer" onclick="showOtherData(this)">Track' + num + '</td><td style="cursor: pointer" onclick="changeName(this)">' + dataT[t].name + '</td><td>' + dataT[t].numPoints + '</td><td>' + dataT[t].len + '</td><td>' + dataT[t].loop + '</td></tr>';
                    $('#gpxViewPanel').append(rowt);
                };
            }

            if (sum === 0) {
                $('#gpxStatusMessage').append('There are NO components within this file.');
            } else {
                $('#gpxStatusMessage').append('There are <span>' + sum + '</span> components on this file.');
            }
        },
        fail: function (error) {
            console.log(error);
        }
    });
}

// adds a new waypoint line to the display for user to input data into.
function addWPT() {
    wptNum++;
    var latlon = '<div class="row"><div class="col-6"><label for="routeLat' + wptNum + '">Lat</label><input type="number" class="form-control" id="routeLat' + wptNum + '" value="43.533330" placeholder="43.533330" step="0.000001"></div><div class="col-6"><label for="routeLon' + wptNum + '">Lon</label><input type="number" class="form-control" id="routeLon' + wptNum + '" value="-80.223610" placeholder="-80.223610" step="0.000001"></div></div>';
    $('#wptlist').append(latlon);
}

// add a route inut form to webpage for user to add a new route to component, 
//calls server to add that route. 
function addRTE() {
    let routeName = $('#routeName').val();
    let file = fileDropDown;
    let wptData = [];
    let error = '';

    for (let n = 1; n <= wptNum; n++) {
        if ($('#routeLon' + n).val() === undefined || $('#routeLat' + n).val() === undefined) {
            error += 'Missing Data: Please enter a valid Lat and Lon!';
        } else if ($('#routeLat' + n).val() < -90.0 || $('#routeLat' + n).val() > 90.0) {
            error += 'Please enter a valid latitude!';
        } else if ($('#routeLon' + n).val() < -180.0 || $('#routeLon' + n).val() > 180.0) {
            error += 'Please enter a valid longitude!';
        }
    }

    if (error !== '') {
        alert(error);
    } else if (routeName.length === 0 || routeName === undefined) {
        alert('Invalid Route Name.');
    } else if (file === '') {
        alert('Please select a route first!');
    } else {
        let routeJSON = '{"name":"' + routeName + '"}';
        for (let n = 1; n <= wptNum; n++) {
            let wptlat = $('#routeLat' + n).val();
            let wptlon = $('#routeLon' + n).val();

            let WPTjson = '{"lat":' + wptlat + ',"lon":' + wptlon + '}';
            wptData.push(WPTjson);
        }
        $.ajax({
            type: "GET",
            url: "/addRoute",
            dataType: "JSON",
            data: {
                wptJSON: wptData,
                routeJSON: routeJSON,
                fileName: file,
                wptNum: wptNum
            },
            success: function (res) {
                updateFileLog()
                updateGPXVeiw();
            },
            fail: function (error) {
                console.log(error);
            }
        });
    }
}

// Basic Login front end logic
//asks user for username, password, and database name
//returns success if credentials match.
function connect() {
    let username = $('#login_username').val();
    let password = $('#login_password').val();
    let dbname = $('#login_dbname').val();
    if (username.length === 0 || username === undefined) {
        alert('Missing Data: Please enter a valid username!');
    } else if (password.length === 0 || password === undefined) {
        alert('Missing Data: Please enter a valid password!');
    } else if (dbname.length === 0 || dbname === undefined) {
        alert('Missing Data: Please enter a valid database name!');
    } else {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/connect',
            data: {
                username: username,
                password: password,
                dbname: dbname
            },
            success: function (res) {
                alert('You have connected to: ' + res['Result']);
                $('#options').show();
                $('#querry').show();
                connected = true;
            },
            fail: function (error) {
                alert('Failed to connect: ' + error);
                $('#options').hide();
                $('#querry').hide();
                connected = false;
                console.log(error);
            }
        });
    }
}

// updates database with files if connected. 
function storeFiles() {
    if (connected === false) {
        alert('Not Connected to Server!!');
    } else {
        $.ajax({
            type: "GET",
            url: "/storeFiles",
            success: function (res) {
                alert('Successfully stored all files into DB!');
            },
            error: function (err) {
                alert('Error: Could not store files into DB');
            }
        });
    }
}

//TODO: updated the server if a change has been made. 
function update() {
    alert('To Be Added!');
}

//clears all files from database. 
function clearData() {
    if (connected === false) {
        alert('Not Connected to Server!!');
    } else {
        $.ajax({
            type: "GET",
            url: "/clearData",
            success: function (res) {
                alert('All data removed from DB! ' + res['Result']);
            },
            error: function (err) {
                alert('Error: Could not remove files from DB');
            }
        });
    }
}

//displays the database onto the webpage. showing how many files, routes and points it has. 
function displayDBStatus() {
    if (connected === false) {
        alert('Not Connected to Server!!');
    } else {
        $.ajax({
            type: "GET",
            url: "/dbstatus",
            dataType: "JSON",
            success: function (res) {
                alert('Database has ' + res['numFiles'] + ' files, ' + res['numRoutes'] + ' routes, and ' + res['numPoints'] + ' points.');
            },
            error: function (err) {
                alert(err);
            }
        });
    }
}

// calls the backend to collect all routes from the database. 
function allRoutes() {
    let num = 0;

    $.ajax({
        type: "GET",
        url: "/query",
        dataType: "JSON",
        data: {
            num: num
        },
        success: function (res) {

        },
        error: function (err) {
            alert('There was a problem contacting DB: ' + err);
        }
    });
}

// calls the backend to get all routes from a particular file in the database. 
function allRoutesFromFile() {
    let num = 2;
    $.ajax({
        type: "GET",
        url: "/query",
        dataType: "JSON",
        data: {
            num: num
        },
        success: function (res) {

        },
        error: function (err) {
            alert('There was a problem contacting DB: ' + err);
        }
    });
}

// calls the backend to gather all points from the database. 
function allPoints() {
    let num = 3;
    $.ajax({
        type: "GET",
        url: "/query",
        dataType: "JSON",
        data: {
            num: num
        },
        success: function (res) {

        },
        error: function (err) {
            alert('There was a problem contacting DB: ' + err);
        }
    });
}

// calls the backend to get all points from a particular file in the database. 
function allPointsFromFile() {
    let num = 4;
    $.ajax({
        type: "GET",
        url: "/query",
        dataType: "JSON",
        data: {
            num: num
        },
        success: function (res) {

        },
        error: function (err) {
            alert('There was a problem contacting DB: ' + err);
        }
    });
}

//TODO: gather the shortest and longest rputes in the database. 
function routesShortLong() {
    let num = 5;
    $.ajax({
        type: "GET",
        url: "/query",
        dataType: "JSON",
        data: {
            num: num
        },
        success: function (res) {

        },
        error: function (err) {
            alert('There was a problem contacting DB: ' + err);
        }
    });
}