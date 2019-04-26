
<?php
	$con = mysqli_connect("localhost","nejoo97","njeunjoo1997!","nejoo97");

	$ID = $_POST["ID"];
	$statement = mysqli_prepare($con, "INSERT INTO MEMBERPOSITION VALUES ($ID, 0, 0)");
	//sssss->문자열형태로 받겠다
	mysqli_stmt_execute($statement);

	$response = array();
	$response["success"] = true; //성공 

	echo json_encode($response); //성공 됨을 응답
?>