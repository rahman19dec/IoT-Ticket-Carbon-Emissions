{
var input = getInputValue("Value in");
// Convert the JSON object to a string to inspect its structure
var jsonString = JSON.stringify(input, null, 2); // Indentation of 2 spaces for readability

// Create an output object and store the JSON string
var output = {};
output.value = jsonString; // Display the entire JSON structure as a string
setOutputValue("Value out", output);

}