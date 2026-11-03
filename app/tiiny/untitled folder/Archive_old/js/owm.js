var myDate = new Date(),
		time = myDate.toLocaleTimeString(),
		month = myDate.getMonth(),
		months = [ "Jan", "Feb", "March", "April", "May", "June", "July", "Aug", "Sept", "Oct", "Nov", "Dec" ],
		today = myDate.getDate(),
		year = myDate.getFullYear();

function getForecast() {
	var cityID = $("#sevenDay").val(),
			multiDay = document.getElementById("multiDay"),
			multipleDay = "https://api.openweathermap.org/data/2.5/forecast/daily?q=" + cityID + "&appid=3f572b62fea31c192fae70757e9de676";
	$.getJSON(multipleDay, weatherCallback);
	function weatherCallback(forecast) {
		

		multiDay.innerHTML = forecast.city.name + "," + forecast.city.country + " <input value= '" + forecast.city.id + "'><br>";
	}
}
