//회원가입 시켜줌
<?php
	$con = mysqli_connect("localhost","nejoo97","njeunjoo1997!","nejoo97");

	$ID = $_POST["ID"];
	$PW = $_POST["PW"];
	$EMAIL = $_POST["EMAIL"];
	$CAR = $_POST["CAR"];
	$GENDER = $_POST["GENDER"];

	$statement = mysqli_prepare($con, "INSERT INTO MEMBER VALUES (?, ?, ?, ?, ?)");
	mysqli_stmt_bind_param($statement, "sssss", $ID, $PW, $EMAIL, $CAR, $GENDER);
	//sssss->문자열형태로 받겠다
	mysqli_stmt_execute($statement);

	$response = array();
	$response["success"] = true; //성공 

	echo json_encode($response); //성공 됨을 응답
?>