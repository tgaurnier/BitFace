
var CONFIG_URL = "https://dl.dropboxusercontent.com/u/38622080/pebbleconfigs/BitFace/configuration.html"

Pebble.addEventListener('ready',
  function(e) {
    console.log('JavaScript app ready and running!');
  }
);


  // register showConfiuration event listner
Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL(CONFIG_URL);
});


// register webviewclosed event listner
Pebble.addEventListener('webviewclosed',
function(e) {
    console.log('Configuration window returned: ' + e.response);
    var configuration = JSON.parse(decodeURIComponent(e.response));
      // Construct a dictionary
  var dict = {
    'KEY_INVERT' : configuration.invert-colours,
    'KEY_DATE_FORMAT' : configuration.date-format,
    'KEY_BATTERY_HIDE' : configuration.battery-hide
  };

  console.log(dict);
  
  // Send a string to Pebble
  Pebble.sendAppMessage(dict,
    function(e) {
      console.log('Send successful.');
    },
    function(e) {
      console.log('Send failed!');
    }
  );
});
