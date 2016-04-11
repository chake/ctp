<?php

/**
*
*/
class Consumer
{

    function __construct()
    {
        $this->mysql = new PDO("mysql:dbname=ctp;host=127.0.0.1", "root", "Abc518131!");
        $this->rds = new Redis();
        $this->rds->connect('127.0.0.1', 6379);
        $this->rds->select(1);
    }

    public function popViaRds($key)
    {
        $data = $this->rds->rPop($key);
        $data = explode('_', $data);
        return $data;
    }

    public function insertDB($sql, $data)
    {
        $st = $pdo->prepare($sql);
        $result = $st->execute($data);
        $re = $pdo->lastInsertId();
        return $re;
    }



}
