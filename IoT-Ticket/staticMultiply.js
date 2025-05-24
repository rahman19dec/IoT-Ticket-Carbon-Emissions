{

var array = getInputValue("array");
var static = getInputValue("static");


var outputArray = [];


    for (var i = 0; i < array.length; i++) {
        var adata = array[i];
        var pdata = null;

	if(adata.value){
      outputArray.push({
        time: adata.time,
        value: adata.value * static[0].value 
      });
    }
    }



var output = outputArray;


setOutputValue("product", output);
}