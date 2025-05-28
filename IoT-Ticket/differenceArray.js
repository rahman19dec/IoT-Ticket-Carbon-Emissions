{
  var array = getInputValue("array");
  var outputArray = [];
  
  for (var i = 1; i < array.length; i++) {
      var prev = array[i - 1];
      var curr = array[i];
  
      if (prev.value !== null && curr.value !== null) {
          outputArray.push({
              time: curr.time,
              value: curr.value - prev.value
          });
      }
  }

  setOutputValue("diff array", outputArray);

}
