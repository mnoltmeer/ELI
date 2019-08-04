#class Cell
{
  #public method Cell($val){&$this.Set($val);}
  
  #public method Set($newval){&$this.Value = $newval;}

  #public method Get(){#return &$this.Value;}
  
  #public method Length(){#return _strlen(&$this.Value);}
  
  #property Value = 0;
}
//------------------------------------------------;

#class Row
{
  #public method Row($cellcount)
  {
    &$this.CreateCells($cellcount);
	&$this.Size = $cellcount;
	&$this.Name = '&' + $this;
  }
//------------------------------------------------;

  #public method Count(){#return &$this.Size;}
//------------------------------------------------;

  #public method Read($ind)
  {
    if (&$this.InRange($ind) == 1)
	  {#return &$this.$ind.Get();}
	else
      {#return err;}	
  }
//------------------------------------------------;

  #public method Write($ind, $val)
  {
    if (&$this.InRange($ind) == 1)
	  {&$this.$ind.Set($val);}
  }
//------------------------------------------------;  
  
  #public method GetText()
  {
    $i = 0;
    $text = sym;

	for ($i, < &$this.Size, +1)
	   {
		 $text = $text + &$this.$i.Get();
		 $text = $text + &$this.Delim;
	   }
    
	#return $text;
  }
//------------------------------------------------;

  #public method SetDelim($delim){&$this.Delim = $delim;}
//------------------------------------------------;
  
  #method CreateCells($count)
  {
    $i = 0;
	
	for ($i, < $count, +1)
	   {
	     &$this.Add($i, #class Cell(0));
	   }
  }
//------------------------------------------------;

  #public method GetName(){#return &$this.Name;}
//------------------------------------------------;  

  #method InRange($pos)
  {
	if ($pos >= &$this.Size)
	  {_throw('Row::out_of_range'); #return 0;}
	else if ($pos < 0)
      {_throw('Row::out_of_range'); #return 0;}
	else
	  {#return 1;}
  }
//------------------------------------------------;  
  
  #property Size = 0;
  #property Delim = ' ';
  #property Name = 0;
}
//------------------------------------------------;


#class RowSet
{
  #public method RowSet($count, $rowsize){&$this.CreateRows($count, $rowsize);}
//------------------------------------------------;

  #public method Count(){#return &$this.RowCount;}
//------------------------------------------------;

  #public method AddRow($row)
  {
	if (&$row.Exist() == 1)
	  {
	    $newind = &$this.Count();
		$newrowsize = &$row.Count();
		
		&$this.Add($newind, #class Row($newrowsize));
		&$this.RowCount = ++1;
	  
	  }  
  }
//------------------------------------------------;

  #public method DeleteRow($ind)
  {
	if (&$this.InRange($ind) != 1)
	  {#return 0;}
	  
	if (&$this.ShiftFrom($ind, 0) == 0)
	  {#return 0;}
	
    &$this.Count = --1;
	
	#return 1;
  }
//------------------------------------------------;

  #public method Insert($pos, $row)
  {
    if (&$this.ShiftFrom($pos, 1) > 0)
	  {
	    $i = 0;
		
		for ($i, <&$row.Count(), +1)
		   {
			 $val = &$row.Read($i);
			 &$this.$pos.Write($i, $val);
		   }
	  }
	else
	  {#return 0;}
  }
//------------------------------------------------;

  #method ShiftFrom($from, $direct)
  {
    if (&$this.InRange($from) != 1)
	  {#return 0;}
    
	if ($direct >= 0)
	  {
		&$this.Add(&$this.Count(), &$this.DefRowSize);	
		$pos = &$this.Count();
		
		for ($pos, >$from, -1)
		  {
			$to = $pos;		 
			$fr = _int($pos - 1);
			&$this.$to = &$this.$fr;
		  }
	  }
	else if ($direct < 0)
	  {
	    $pos = $from;
		
		for ($pos, < &$this.LastInd(), +1)
		  {
			$to = [$pos];		 
			$fr = [_int($pos + 1)];
			&$this.$to = &$this.$fr;
		  }
		
		$el = [&$this.LastInd()];
		
		&$this.Remove($el);
		
	  }
	  
	#return 1;   
  }
//------------------------------------------------;

  #method CreateRows($count, $rowsize)
  {
    $i = 0;
	
	for ($i, < $count, +1)
	   {
	     &$this.Add($i, #class Row($rowsize));
	   }
	   
	&$this.RowCount = $count;
	&$this.DefRowSize = $rowsize;
  }
//------------------------------------------------;

  #method InRange($pos)
  {
	if ($pos >= &$this.RowCount)
	  {_throw('RowSet::out_of_range'); #return 0;}
	else if ($pos < 0)
      {_throw('RowSet::out_of_range'); #return 0;}
	else
	  {#return 1;}
  }
//------------------------------------------------;
  
  #property RowCount = 0;
  #property DefRowSize = 0;
}