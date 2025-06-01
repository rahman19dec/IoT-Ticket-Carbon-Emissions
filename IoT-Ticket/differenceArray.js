var array = getInputValue("array");
var grouped = {};
var outputArray = [];

// External compare function to avoid inline function in loop
function sortByTimeAsc(a, b) {
    return new Date(a.time) - new Date(b.time);
}

// Group items by source
for (var i = 0; i < array.length; i++) {
    var item = array[i];
    var source = item.source;

    if (!grouped[source]) {
        grouped[source] = [];
    }
    grouped[source].push(item);
}

// Compute deltas, keep original objects
for (var source in grouped) {
    var sourceArray = grouped[source];

    sourceArray.sort(sortByTimeAsc); // Sort by time

    for (var i = 1; i < sourceArray.length; i++) {
        var prev = sourceArray[i - 1];
        var curr = sourceArray[i];

        if (prev.value !== null && curr.value !== null) {
            var diffItem = Object.assign({}, curr); // clone original object
            diffItem.value = curr.value - prev.value;
            outputArray.push(diffItem);
        }
    }
}

setOutputValue("diff array", outputArray);
