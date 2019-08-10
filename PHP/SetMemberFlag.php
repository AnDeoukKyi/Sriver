<?php
 	header("Connect-Type: text/html; charset=UTF-8");
	$con = mysqli_connect("localhost","nejoo97","njeunjoo1997!","nejoo97");

    	$ID = $_GET["id"];
    	$FLAG = $_GET["FLAG"];

	$statement = mysqli_prepare($con, "UPDATE MEMBER SET FLAG = $FLAG WHERE ID = '$ID'");


	$a = mysqli_stmt_execute($statement);

    mysqli_close($con);
?>

