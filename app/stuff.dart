const char* STA_Root_HTML_URL = "https://prjmjennings.co.uk/assets/cq/app/sta_root.html";
const char* AP_Root_HTML_URL = "https://prjmjennings.co.uk/assets/cq/app/ap_root.html";


void ConfigWebServer()
{
  // Route to serve the root page, with WiFi configuration form
  server.on("/", handleRoot);  // This serves the main page with the form
  
  // Handle WiFi connection request (form submission)
  server.on("/connect", HTTP_POST, handleConnect); // This handles the POST request from the form
  
  // Handle the factory reset request
  server.on("/factoryreset", factoryreset);  // This resets the settings
  
  // Handle any unknown routes
  server.onNotFound(handleNotFound);  // This serves a "Not Found" page for invalid routes

  // Start the web server
  server.begin();
  Serial.println("HTTP server started");
}

void setup() 
{
  EEPROM.begin(512);
  Serial.begin(115200);
  delay(500);  // Wait for serial startup
  
  timeClient.begin();
  StartupNeopixels();

  pinMode(ldrSensor, INPUT);
  mood = 0;  // Default mood value

  GetConfiguration(); 

  // Set the WiFi mode to AP and STA
  WiFi.mode(WIFI_AP_STA);
  StartupAccesspoint(); 

  // Attempt to connect to WiFi
  connectWifi();

  // Wait for the WiFi connection to be established
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
    ShowMyIP();  // Show the local IP address
  } else {
    Serial.println("Failed to connect to WiFi");
    // You might want to reset or handle the failure here if necessary
  }

  // Start the web server after ensuring WiFi connection
  ConfigWebServer();

  // Only continue with the rest of the setup if WiFi is connected
  if (WiFi.status() == WL_CONNECTED) {
    CalculateDST();   // First: calculate daylight savings time
    GetNtpTime();     // Second: get NTP time
    getWeatherFirst(); // Get initial weather update
    ShowMyIP();        // Show IP on the clock display
  }

  Serial.println("Startup done");
}

// Function to fetch HTML content from the specified URL
String fetchExternalHTML(const char* url) {
  HTTPClient http;  // Create an HTTPClient instance
  
  // Initialize HTTP connection
  http.begin(url);  // Begin the connection to the given URL
  
  // Send HTTP GET request
  int httpCode = http.GET();  // Get the response code
  
  String payload = "";  // Initialize a string to hold the HTML content
  
  if (httpCode > 0) {  // Check if the GET request was successful
    // If the request is successful, get the HTML content
    payload = http.getString();  // Get the response body as a string
    Serial.println("HTML content fetched successfully.");
  } else {
    // If there's an error, print the error code
    Serial.println("Error fetching HTML content, HTTP code: " + String(httpCode));
  }
  
  http.end();  // Close the HTTP connection
  
  return payload;  // Return the HTML content as a string
}

void handleRoot() {
  String htmlContent = "";

  // Check if we are in AP mode or STA mode and select the corresponding HTML file
  if (server.client().localIP() == Accesspoint_IP) {
    Serial.println("* Handling AP Mode Root *");
    htmlContent = fetchExternalHTML(AP_Root_HTML_URL);  // Fetch the AP HTML file
  } else if (server.client().localIP() == WiFi.localIP()) {
    Serial.println("* Handling STA Mode Root *");
    htmlContent = fetchExternalHTML(STA_Root_HTML_URL);  // Fetch the STA HTML file
  }

  // Check if the HTML content was successfully fetched
  if (htmlContent.length() > 0) {
    
    // Create dynamic list of available SSIDs
    String ssidList = "";
    byte numSsid = WiFi.scanNetworks();
    
    if (numSsid == 0) {
      ssidList += "<li>No networks found</li>";
    } else {
      for (int i = 0; i < numSsid; i++) {
        String this_ssid = WiFi.SSID(i);
        ssidList += "<li><input type='radio' id='" + this_ssid + "' name='ssid1' value='" + this_ssid + "'>";
        ssidList += "<label for='" + this_ssid + "'>" + this_ssid + "</label>";
        ssidList += "<div class='check coral coral__'></div></li>";
      }
    }

    // Replace the placeholders with dynamic values
    htmlContent.replace("{{URL}}", String(URL));
    htmlContent.replace("{{seconds_status}}", String(seconds_status));
    htmlContent.replace("{{brightness}}", String(brightness));
    htmlContent.replace("{{ldrSensor_status}}", String(ldrSensor_status));
    htmlContent.replace("{{sensor_timer}}", String(sensor_timer));
    htmlContent.replace("{{motionSensor}}", String(motionSensor));
    htmlContent.replace("{{cityName}}", String(cityName));
    htmlContent.replace("{{time_mode}}", String(time_mode));
    htmlContent.replace("{{cool}}", String(cool));
    htmlContent.replace("{{warm}}", String(warm));
    htmlContent.replace("{{hot}}", String(hot));
    htmlContent.replace("{{ambience}}", String(ambience));
    htmlContent.replace("{{show_weather}}", String(show_weather));
    htmlContent.replace("{{summer_offset}}", String(summer_offset));
    htmlContent.replace("{{winter_offset}}", String(winter_offset));

    htmlContent.replace("{{wifi_ssid_options}}", ssidList);  // Insert SSID list

    // Send the modified HTML as the response
    server.send(200, "text/html", htmlContent);
  } else {
    server.send(500, "text/plain", "Error fetching HTML content.");
  }
}

void handleConnect() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  Serial.println("Connecting to: " + ssid);
  WiFi.begin(ssid.c_str(), password.c_str());

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected!");
    server.send(200, "text/html", "<html><body>Connected to " + ssid + "</body></html>");
  } else {
    server.send(500, "text/html", "<html><body>Failed to connect to WiFi</body></html>");
  }
}