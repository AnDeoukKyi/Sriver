<?php
 	header("Connect-Type: text/html; charset=UTF-8");
	$con = mysqli_connect("localhost","nejoo97","njeunjoo1997!","nejoo97");


	$result = mysqli_query($con, "SELECT ID FROM MEMBER WHERE FLAG = 1");


	$response = array();
    	while($row = mysqli_fetch_array($result)){
        	array_push($response, array("ID"=>$row[0]));
    	}

 	echo json_encode(array("response"=>$response), JSON_UNESCAPED_UNICODE);
    mysqli_close($con);
?>

