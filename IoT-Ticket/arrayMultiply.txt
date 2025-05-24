{
// Version: Ignore indoor items with no time match in outdoor

// Get the input arrays
var indoor = getInputValue("array 1");
var outdoor = getInputValue("array 2");

// Initialize an empty array for the output
var outputArray = [];

// Assume indoor and outdoor are arrays, per "no validation" request
if (indoor && outdoor) {
    // Loop through the indoor array
    for (var i = 0; i < indoor.length; i++) {
        var indoorData = indoor[i];
        var matchedOutdoorData = null;

        // Find the matching outdoor data based on the time field
        for (var j = 0; j < outdoor.length; j++) {
            // Check if time properties exist before comparing (minimal check)
             if (outdoor[j] && indoorData && outdoor[j].time === indoorData.time) {
                 matchedOutdoorData = outdoor[j];
                 break; // Found match, exit inner loop
             }
        }

        // If a matching time was found, calculate the difference and add to the output
        if (matchedOutdoorData.value && indoorData.value) {
            // Assume .value exists and is numeric, per "no validation" request
             outputArray.push({
                time: indoorData.time, // Preserve the time field
                value: indoorData.value * matchedOutdoorData.value // Calculate the difference
            });
        }
        // If no match (matchedOutdoorData is null), do nothing (ignore)
    }
} else {
     console.error("Input 'value1' or 'value2' is missing or not an array.");
}


// Prepare the output object
var output = outputArray;

// Set the output value
setOutputValue("product", output);
}