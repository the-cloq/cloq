function getColorFromURL() {
        const urlParams = new URLSearchParams(window.location.search);
        color = urlParams.get('color') || "red";  // Default to red if not provided
        scolor = urlParams.get('scolor') || "green";  // Default to green if not provided

        // Additional variables for checkboxes
        seconds_status = urlParams.get('seconds_status') || "0";  // Default to "0" if not provided
        ldrSensor_status = urlParams.get('ldrSensor_status') || "0";  // Default to "0" if not provided
        motionSensor = urlParams.get('motionSensor') || "0";  // Default to "0" if not provided
        time_mode = urlParams.get('time_mode') || "default";  // Default to "default" if not provided
        Temp_mode = urlParams.get('Temp_mode') || "cel";  // Default to "cel" if not provided
    }

    function setCheckedState() {
        // Color radio buttons
        if (document.getElementById(color)) document.getElementById(color).checked = true;
        if (document.getElementById(scolor)) document.getElementById(scolor).checked = true;

        // Checkboxes
        if (document.getElementById('seconds_status')) {
            document.getElementById('seconds_status').checked = seconds_status === "1";
        }
        if (document.getElementById('ldrSensor_status')) {
            document.getElementById('ldrSensor_status').checked = ldrSensor_status === "1";
        }
        if (document.getElementById('motionSensor')) {
            document.getElementById('motionSensor').checked = motionSensor === "1";
        }
        if (document.getElementById('time_mode')) {
            document.getElementById('time_mode').checked = time_mode === "city_local";
        }

        // Temp mode radio buttons
        if (document.getElementById('celcius')) {
            document.getElementById('celcius').checked = Temp_mode === "cel";
        }
        if (document.getElementById('fahrenheit')) {
            document.getElementById('fahrenheit').checked = Temp_mode === "fah";
        }
    }

    window.onload = function() {
        getColorFromURL();  // Get values from the URL
        setCheckedState();  // Set the checked state for the inputs
    };