<?php
 	header("Connect-Type: text/html; charset=UTF-8");
	$con = mysqli_connect("localhost","nejoo97","njeunjoo1997!","nejoo97");

    	$ID = $_GET["id"];

	$statement = mysqli_prepare($con, "UPDATE MEMBER SET FLAG = 2 WHERE ID = '$ID'");


	$a = mysqli_stmt_execute($statement);

	$response = array();
	$response["success"] = true; //성공 
	$response["a"] = $a; //성공 
	$response["statement"] = $statement; //성공 

 	echo json_encode($response); //성공 됨을 응답
    mysqli_close($con);
?>

