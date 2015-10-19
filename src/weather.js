var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

var forecast = "Upd";
var forecasttemp = "Upd";
var temperature = "Upd";
var conditions = "Upd";
var city = "Upd";
var text = "Upd";

function locationSuccess(pos) {  
  var urlf = 'http://api.openweathermap.org/data/2.5/forecast?Lat='+pos.coords.latitude +'&Lon=' + pos.coords.longitude +'&units=metric&mode=json&APPID=38c8da0e62b8ef98c45e1e3436950da8';
  var url = 'http://api.openweathermap.org/data/2.5/weather?Lat='+pos.coords.latitude +'&Lon=' + pos.coords.longitude +'&units=metric&mode=json&APPID=38c8da0e62b8ef98c45e1e3436950da8';
  
	console.log("Lat is " + pos.coords.latitude);
	console.log("Lon is " + pos.coords.longitude);

  // Send request to openweathermap.org
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
			//console.log("responsetext is " + json);

      // °F to °C
      //var temperature = Math.round(json.currently.apparentTemperature); // ((json.currently.temperature-32)*5)/9
      //console.log("Temperature is " + temperature);
			
      temperature = Math.round((json.main.temp));
      console.log("Temperature is " + temperature);
      
      city = json.name;
      console.log ("City is " + city);
      

			// Conditions
      conditions = json.weather[0].icon;
      console.log("Conditions are " + conditions);
      
      text = json.weather[0].description;
      console.log("Text is " + text);
      
      xhrRequest(urlf, 'GET',
        function(responseText) {   
          var json = JSON.parse(responseText);
          forecast = json.list[2].weather[0].icon;
          forecasttemp = Math.round(json.list[2].main.temp);
          console.log ("Forecast is " + forecast);
          console.log ("Forecast temp is: " + forecasttemp);

      
        // Assemble dictionary using our keys
        var dictionary = {
          "KEY_TEMPERATURE": temperature,
          "KEY_CONDITIONS": conditions,
          "KEY_FORECASTC": forecast,
          "KEY_CITY": city,
          "KEY_FORECASTT": forecasttemp,
          "KEY_DESC": text,
        };

        // Send to Pebble
        Pebble.sendAppMessage(dictionary,
          function(e) {
            console.log("Weather info sent to Pebble successfully!");
          },
          function(e) {
            console.log("Error sending weather info to Pebble!");
          }
        );
      }
    );
      }  
   );
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 30000}
  );
}



// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');

    // Get the initial weather
    getWeather();
    
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
    
  }                     
);
