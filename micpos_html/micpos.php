<!DOCTYPE html>
<html>
<head>
<meta http-equiv="refresh" content="1" >
<meta charset="ISO-8859-1">
<title>Team 4</title>
</head>
<body>
<?php 
echo $_POST['Shares']; 
var_dump($_POST);
print_r($_POST);
extract($POST);
?>
<li><b>MICPOS Projekt</b></li>
<li>Team 4: Andreas Mikula, Daniel Schinewitz, Ibrahim Milli, Philip Albrecht</li>
<style>

	div.bereich {
		width: 400px;
		height: 300px;
		padding: 40px;
		margin: 30px;
		border: 3px green solid;
		position: fixed;
	}

	div.circle {
		border: 3px solid red;
		border-radius: 50%;
		width: 60px;
		height: 60px;
		margin-left: 160px;
		margin-top: 20px;
		position: fixed;
	}

	.circle span {
		font-size: 10px;
		font-weight: bold;
		text-transform: uppercase;
		display: table-cell;
		text-align: center;
		horizontal-align: middle;
		vertical-align: middle;
		padding: 20px;
	}

	div.circle1 {
		border: 3px solid red;
		border-radius: 50%;
		width: 60px;
		height: 60px;
		left: 120px;
		top: 345px;
		position: fixed;
	}

	.circle1 span {
		font-size: 10px;
		font-weigth: bold;
		text-transform: uppercase;
		display: block;
		text-align: center;
		horizontal-align: middle;
		vertical-align: middle;
		padding: 20px;
	}

	div.circle2 {
		border: 3px solid red;
		border-radius: 50%;
		width: 60px;
		height: 60px;
		left: 350px;
		top: 345px;
		position: fixed;
	}

	.circle2 span {
		font-size: 10px;
		font-weight: bold;
		text-transform: uppercase;
		display: block;
		text-align: center;
		horizontal-align: middle;
		vertical-align: middle;
		padding: 20px;
	}
</style>
<div class=bereich >
<div id="tiva1" class=circle> <span> Tiva 1 </span></div>
<div id="tiva2" class=circle1><span> Tiva 2 </span></div>
<div id="tiva3" class=circle2><span> Tiva 3 </span></div>
</div>

<script type="text/javascript" >


var input;

function getparameter (input) {
if(input === 1){
	document.getElementById("tiva1").style.backgroundColor = "lightgreen";
	}
else if (input == 2) {
	document.getElementById("tiva2").style.backgroundColor = "lightgreen";
	}
else if (input == 3) {
	document.getElementById("tiva3").style.backgroundColor = "lightgreen";
	}
else
	{
	exit;
	}
}

getparameter("1");
//getparameter($_POST['Shares']);
</script>
</body>
</html>
