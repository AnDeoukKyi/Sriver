//id,pw중복체크
<?php
	$con = mysqli_connect("localhost","nejoo97","njeunjoo1997!","nejoo97");

	$POSX = $_POST["POSX"]; //입력
	$POSY = $_POST["POSY"];
	

	$statement = mysqli_prepare($con, "UPDATE MEMBERPOS SET POSX = ?, POSY = ? WHERE ID = '1234'"); 
	mysqli_stmt_bind_param($statement, "ss", $POSX, $POSY); 
	//sssss->문자열형태로 받겠다
	mysqli_stmt_execute($statement);

	$response = array();
	$response["success"] = true;  


	echo json_encode($response); //성공 됨을 응답
?>