<?php
    header("Connect-Type: text/html; charset=UTF-8");
    $con = mysqli_connect("localhost", "nejoo97", "njeunjoo1997!", "nejoo97");


    $ID = $_GET["ID"];
    $POSX = $_GET["posx"];
    $POSY = $_GET["posy"];

    $result = mysqli_query($con, "UPDATE MEMBER SET POSX = $POSX , POSY = $POSY WHERE ID = '$ID'");


    mysqli_stmt_bind_param($result, "iiii", $index, $occupy, $startX, $startY);
    mysqli_stmt_execute($result);



    mysqli_close($con);
?>



