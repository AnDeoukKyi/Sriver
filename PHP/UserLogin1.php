//id,pw중복체크
<?php
	$con = mysqli_connect("localhost","nejoo97","njeunjoo1997!","nejoo97");

	$ID = $_GET["ID"]; //입력
	$PW = $_GET["PW"];
	

	$statement = mysqli_prepare($con, "SELECT ID FROM MEMBER WHERE ID  = ? AND PW = ? "); //현재 입력받은 id가 이미 존재 하는지 확인
	mysqli_stmt_bind_param($statement, "ss", $ID, $PW); 
	//sssss->문자열형태로 받겠다
	mysqli_stmt_execute($statement);

	mysqli_stmt_store_result($statement);
	mysqli_stmt_bind_result($statement, $ID);

	$response = array();
	$response["success"] = false;  

	while (mysqli_stmt_fetch($statement)) { //같은 id 있으면 안성공
		$response["success"] = true;
		$response["ID"] = $ID;
		
	}

	echo json_encode($response); //성공 됨을 응답
?>