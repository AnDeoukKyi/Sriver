<?php
	$con = mysqli_connect("localhost","nejoo97","njeunjoo1997!","nejoo97");

	$ID = $_GET["ID"]; 
	

	$statement = mysqli_query($con, "SELECT POSX, POSY, DIR FROM MEMBER WHERE ID  = '$ID'"); //현재 입력받은 id가 이미 존재 하는지 확인

	$response = array();
	while($row = mysqli_fetch_array($statement)){
        array_push($response, array("POSX"=>$row[0], "POSY"=>$row[1], "DIR"=>$row[2]));
    }
	echo json_encode(array("response"=>$response), JSON_UNESCAPED_UNICODE);
    mysqli_close($con);
?>