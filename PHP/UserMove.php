<?php
    header("Connect-Type: text/html; charset=UTF-8");
    $con = mysqli_connect("localhost", "nejoo97", "njeunjoo1997!", "nejoo97");


    $ID = $_GET["id"];
    $POSX = $_GET["POSX"];
    $POSY = $_GET["POSY"];

    $result = mysqli_query($con, "UPDATE MEMBER SET POSX = $POSX , POSY = $POSY WHERE ID = '$ID'");


    mysqli_stmt_execute($result);



    mysqli_close($con);
?>



