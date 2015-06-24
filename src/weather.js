var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  // Construct URL
	//var url = 'https://api.forecast.io/forecast/apikey/' + 
	//pos.coords.latitude + ',' + pos.coords.longitude;
	
  
  var url = 'http://api.openweathermap.org/data/2.5/forecast?Lat='+pos.coords.latitude +'&Lon=' + pos.coords.longitude +'&mode=json';
  
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
			
      var temperature = Math.round((json.list[0].main.temp - 273.15));
      console.log("Temperature is " + temperature);
      
      var city = json.city.name;
      console.log ("City is " + city);
      
			//var temperaturec = Math.round((json.currently.apparentTemperature - 32) * 5/9);
			//console.log("Temperature in C is " + temperaturec);

      // Conditions
      //var conditions = json.minutely.summary;      
      //console.log("Conditions are " + conditions);
			// Conditions
      var conditions = json.list[0].weather[0].icon;
      console.log("Conditions are " + conditions);
      
      var forecast = json.list[1].weather[0].icon;
      console.log ("Forecast is " + forecast);
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_CONDITIONS": conditions,
        "KEY_FORECASTC": forecast,
        "KEY_CITY": city,
				//"KEY_TEMPERATUREC": temperaturec,
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

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
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
