<?php
 	header("Connect-Type: text/html; charset=UTF-8");
	$con = mysqli_connect("localhost","nejoo97","njeunjoo1997!","nejoo97");

    $ID = $_GET["ID"];
    $FLAG = $_GET["FLAG"];

	$statement = mysqli_prepare($con, "UPDATE MEMBER SET FLAG = $FLAG WHERE ID = '$ID'");


	mysqli_stmt_execute($statement);
	mysqli_stmt_store_result($statement);
	mysqli_stmt_bind_result($statement,$ID);

	$response = array();
	$response["success"] = true; //성공 

 	echo json_encode($response); //성공 됨을 응답
    mysqli_close($con);
?>

