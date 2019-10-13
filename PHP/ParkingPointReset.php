<?php
 	header("Connect-Type: text/html; charset=UTF-8");
	$con = mysqli_connect("localhost","nejoo97","njeunjoo1997!","nejoo97");

    $POINT = $_GET["INDEX"];

	$statement = mysqli_prepare($con, "UPDATE PARKINGPOINT SET ID = 'NULL', OCCUPY = 0 WHERE POINT = $POINT");


	mysqli_stmt_execute($statement);
	mysqli_stmt_store_result($statement);
	mysqli_stmt_bind_result($statement,$ID);

	$response = array();
	$response["success"] = true; //성공 

 	echo json_encode($response); //성공 됨을 응답
    mysqli_close($con);
?>

