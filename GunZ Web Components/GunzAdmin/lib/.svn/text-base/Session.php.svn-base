<?php


class Session
{
  var $session_name;
  var $existance;

  function Session ($session_name)
  {
    $this->session_name = $session_name;
    $this->existance = false;
  }

  function start ()
  {
    session_start ();
    $this->existance = true;
  }

  function exist ()
  {
    return $this->existance;
  }

  function isRegister ($session_name)
  {
    if ($_SESSION[$session_name] != "")
    {
      return true;
    }
    else return false;
  }

  function register ($session_name, $session_value)
  {
    if (!$this->isRegister ($session_name))
    {
      session_register ($session_name);
    }

    $_SESSION[$session_name] = $session_value;
  }

  function get ($session_name)
  {
    if ($this->exist () && $this->isRegister ($session_name))
    {
      return $_SESSION[$session_name];
    }
  }

  function clear ($session_name)
  {
    if ($this->isRegister ($session_name))
    {
      @session_unregister ($session_name);
    }
  }

  function destroy ()
  {
    if ($this->exist ())
    {
      @session_destroy ();
    }
  }

}
?>
