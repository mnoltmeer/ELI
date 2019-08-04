#class List
{
  #public method List($size)
  {
    if ($size > 0)
	  {
	    count ($size)
		   {&$this.Push(0);}
	  }
  }
//------------------------------------------------;
  
  #public method Size(){#return &$this.Count;}
//------------------------------------------------;  
  
  #public method Clear()
  {
    $i = 0;
	
	for ($i, <&$this.Count, +1)
	   {&$this.Remove([ + $i + ]);}
	   
	&$this.Count = 0;
	
	#return 1;
  }
//------------------------------------------------;
  
  #public method Get($ind)
  {
	if (&$this.InRange($ind) == 1)
	  {
	    $prop = [ + $ind + ];
		#return &$this.$prop;
	  }  
  }
//------------------------------------------------;
  
  #public method Set($ind, $element)
  {
    if (&$this.InRange($ind) == 1)
	  {
	    $prop = [ + $ind + ]; 
		&$this.$prop = $element;
		#return 1;
	  }
	else
      {#return 0;}	
  }
//------------------------------------------------;
   
  #public method Last()
  {
    $prop = [ + &$this.LastInd() + ];
	#return &$this.$prop;
  }
//------------------------------------------------;
  
  #public method First(){#return &$his.[0];}
//------------------------------------------------;
  
  #public method Push($element)
  {
	$ind = [ + _int(&$this.Count) + ];
	&$this.Add($ind, $element);
	&$this.Count = ++1;
	
	#return 1;
  }
//------------------------------------------------;
  
  #public method Delete($ind)
  {
	if (&$this.InRange($ind) != 1)
	  {#return 0;}
	  
	if (&$this.ShiftFrom($ind, -1) == 0)
	  {#return 0;}
	
    &$this.Count = --1;
	
	#return 1;
  }
//------------------------------------------------;
  
  #public method Insert($pos, $element)
  {
    if (&$this.ShiftFrom($pos, 1) > 0)
	  {#return &$this.Set($pos, $element);}
	else
	  {#return 0;}
  }
//------------------------------------------------;
  
  #method InRange($pos)
  {
	if ($pos >= &$this.Count)
	  {_throw('List::out_of_range'); #return 0;}
	else if ($pos < 0)
      {_throw('List::out_of_range'); #return 0;}
	else
	  {#return 1;}
  }
//------------------------------------------------;
  
  #method ShiftFrom($from, $direct)
  {
    if (&$this.InRange($from) != 1)
	  {#return 0;}
    
	if ($direct >= 0)
	  {
		&$this.Push(0);	
		$pos = &$this.LastInd();
		
		for ($pos, >$from, -1)
		  {
			$to = [ + $pos + ];		 
			$fr = [ + _int($pos - 1) + ];
			&$this.$to = &$this.$fr;
		  }
	  }
	else if ($direct < 0)
	  {
	    $pos = $from;
		
		for ($pos, < &$this.LastInd(), +1)
		  {
			$to = [ + $pos + ];		 
			$fr = [ + _int($pos + 1) + ];
			&$this.$to = &$this.$fr;
		  }
		
		$el = [&$this.LastInd()];
		
		&$this.Remove($el);
		
	  }
	  
	#return 1;   
  }
//------------------------------------------------;
  
  #method LastInd()
  {
	if (&$this.Count > 0)
	  {$lastind = &$this.Count - 1; #return $lastind;}
	else
	  {#return 0;}
  }
//------------------------------------------------;

  #property Count = 0;
}