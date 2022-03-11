#include <stdio.h>

#include "crossword.h"
#include "crosswordhtml.h"

static char *crossword_js;

int crosswordhtml(char *fname, CROSSWORD *cw, int *err)
{
  FILE *fp;

  fp = fopen(fname, "w");
  if(!fp)
  {
    *err = -2;
    return -1;
  }
  fcrosswordhtml(fp, cw);
  fclose(fp);
  *err = 0;

  return 0;
}

int fcrosswordhtml(FILE *fp, CROSSWORD *cw)
{
  int i, ii;

  fprintf(fp, "<HTML>\n");
  fprintf(fp, "<HEAD>\n");
  fprintf(fp, "<TITLE> Crossword </TITLE>\n");
  fprintf(fp, "</HEAD>\n");

  fprintf(fp, "<BODY bgcolor = #FFFFFF>\n");
  fprintf(fp, "<TABLE border = 1 cellspacing = 1 bgcolor = #000000>\n");
  for(i=0;i<cw->height;i++)
  {
    fprintf(fp, "<TR>\n");
	for(ii=0;ii<cw->width;ii++)
	{
	   if(cw->grid[i*cw->width+ii] == 0)
	   {
		   fprintf(fp, "<TD width = 24 height = 24 bgcolor = #000000> </TD>\n");
	   }
	   else
	   {
	     if(cw->numbers[i*cw->width+ii] == 0)
           fprintf(fp, "<TD width = 24 height = 24 bgcolor = #FFFFFF> </TD>\n");
		 else
		 {
		   fprintf(fp, "<TD width = 24 height = 24 bgcolor = #FFFFFF>");
		   fprintf(fp, "<sup><sup>%d</sup></sup>", cw->numbers[i*cw->width+ii]);
		   fprintf(fp, "</TD>\n");
		 }
	   }
	}
	fprintf(fp, "</TR>\n");
  }
  fprintf(fp, "</TABLE>\n");
  fprintf(fp, "<TABLE width = %d>\n", 25 * cw->width);
  fprintf(fp, "<THEAD> <TH width = \"50%%\"> Across </TH> <TH width = \"50%%\"> Down </TH> </THEAD>\n");
  fprintf(fp, "<TBODY>\n");
  fprintf(fp, "<TD valign = \"top\">\n");
  for(i=0;i<cw->Nacross;i++)
  {
    fprintf(fp, "<B>%d.</B> ", cw->numbersacross[i]);
	fprintf(fp, "%s<BR>\n", cw->cluesacross[i]);
  }
  fprintf(fp, "</TD>\n");
  fprintf(fp, "<TD valign = \"top\">\n");
  for(i=0;i<cw->Ndown;i++)
  {
    fprintf(fp, "<B>%d.</B> ", cw->numbersdown[i]);
	fprintf(fp, "%s<BR>\n", cw->cluesdown[i]);
  }
  fprintf(fp, "</TD>\n");
  fprintf(fp, "</TBODY>\n");
  fprintf(fp, "</TABLE>\n");
  fprintf(fp, "</BODY>\n");

  fprintf(fp, "</HTML>\n");

  return 0;

}

int crosswordinteractivehtml(char *fname, CROSSWORD *cw, int *err)
{
  FILE *fp;

  fp = fopen(fname, "w");
  if(!fp)
  {
    *err = -2;
	return -1;
  }
  fcrosswordinteractivehtml(fp, cw); 
  fclose(fp);
  return 0;

}

int fcrosswordinteractivehtml(FILE *fp, CROSSWORD *cw)
{
	int x, y;
	int i;
    int checksum = 100;

	fprintf(fp, "<html>\n");
	fprintf(fp, "<head>\n");
	//fprintf(fp, "<script src = \"crossword.js\"> </script>\n");
	fprintf(fp, "<script type = \"text/javascript\">\n");
	fputs(crossword_js, fp);
	fprintf(fp, "function loadMe() {\n");
    //fprintf(fp, "alert(\"Loaded\");\n");
	fprintf(fp, "cw = new crossword(\n");
	for(y=0;y<cw->height;y++)
	{
		fprintf(fp, "           \"");
		for(x=0;x<cw->width;x++)
			fprintf(fp, "%c", cw->grid[y*cw->width+x] ? 'X' : '.');
		if(y < cw->height -1)
		  fprintf(fp, "\" + \n");
		else
		  fprintf(fp, "\", \n");
	}
    fprintf(fp, "%d, %d, %d);\n", cw->width, cw->height, checksum); 
    fprintf(fp, "cw.setUpTable();\n");
	fprintf(fp, "}\n");
    fprintf(fp, "</script>\n");
    fprintf(fp, "</head>\n");
    fprintf(fp, "<body onkeydown=\"GetChar (event);\" onload = \"loadMe();\">\n");
	fprintf(fp, "<table id = 'myCrossword', bgcolor = \"#000000\">\n");
    fprintf(fp, "</table>\n");


	fprintf(fp, "<table width = %d>\n", 25 * cw->width);
  fprintf(fp, "<thead> <th width = \"50%%\"> Across </th> <th width = \"50%%\"> Down </th> </thead>\n");
  fprintf(fp, "<tbody>\n");
  fprintf(fp, "<td valign = \"top\">\n");
  for(i=0;i<cw->Nacross;i++)
  {
    fprintf(fp, "<b>%d.</b> ", cw->numbersacross[i]);
	fprintf(fp, "%s<br>\n", cw->cluesacross[i]);
  }
  fprintf(fp, "</td>\n");
  fprintf(fp, "<td valign = \"top\">\n");
  for(i=0;i<cw->Ndown;i++)
  {
    fprintf(fp, "<b>%d.</b> ", cw->numbersdown[i]);
	fprintf(fp, "%s<br>\n", cw->cluesdown[i]);
  }
  fprintf(fp, "</td>\n");
  fprintf(fp, "</tbody>\n");
  fprintf(fp, "</table>\n");

    fprintf(fp, "</body>\n");
	fprintf(fp, "</html>\n");

	return 0;
}

static char *crossword_js = "/*\n"
"  check is a single-char string is an upper case letter\n"
"*/\n"
"function isupper(ch)\n"
"{\n"
"  var val = ch.charCodeAt(0);\n"
"  if(val >= 65 && val <= 90)\n"
"    {return true;}\n"
"  else\n"
"    {return false;}\n"
"}\n"
"\n"
"/*\n"
"  generate a list of square numbers for a puzzle\n"
"  Params: puzzle - the puzzle. Filled squares must return true for isalpha()\n"

"          width - puzzle width\n"
"\t\t  height - puzzle height\n"
"  Returns list of numbers (most will be zero)\n"
"*/\n"
"function generatenumbers(puzzle, width, height)\n"
"{\n"
"  var answer;\n"
"  var north;\n"
"  var south;\n"
"  var east;\n"
"  var west;\n"
"  var i, ii;\n"
"  var j = 1;\n"
"\n"
"  answer = new Array();\n"
"  for(i=0;i<height;i++)\n"
"  {\n"
"    for(ii=0;ii<width;ii++)\n"
"    {\n"
"        answer[i*width+ii] = 0;\n"
"\t  north = 0;\n"
"\t  south = 0;\n"
"\t  west = 0;\n"
"\t  east = 0;\n"
"\t  if(! isupper( puzzle[i*width+ii]))\n"
"        {\n"
"\t    continue;\n"
"        }\n"
"\t  if(i == 0 || ! isupper( puzzle[(i-1)*width+ii]) )\n"
"        {\n"
"\t\tnorth = 1;\n"
"        }\n"
"\t  if(i == height -1 || ! isupper(puzzle[(i+1)*width+ii]))\n"
"        {\n"
"\t    south = 1;\n"
"        }\n"
"\t  if(ii == 0 || !isupper(puzzle[i*width+ii-1]))\n"
"        {\n"
"          west = 1;\n"
"        }\n"
"\t  if(ii == width -1 || ! isupper(puzzle[i*width+ii+1]))\n"
"        {\n"
"\t    east = 1;\n"
"        }\n"
"\t  if(north == 1 && south == 0)\n"
"        {\n"
"\t    answer[i*width+ii] = j++;\n"
"        }\n"
"\t  else if(west == 1 && east == 0)\n"
"        {\n"
"\t    answer[i*width+ii] = j++;\n"
"        }\n"
"\t}\n"
"    }\n"
"  return answer;\n"
"}\n"
"\n"
"\n"
"/*\n"
"  fletcher hash function for checksum.\n"
"  This version operates on a strign and returns a number\n"
"*/\n"
"\n"
"function fletcher( data )\n"
"{\n"
"  var sum1 = 0xffff;\n"
"  var sum2 = 0xffff;\n"
"  var tlen;\n"
"  var len = data.length;\n"
"  var i = 0;\n"
"\n"
"  while (len) \n"
"  {\n"
"    if(len > 360) {tlen = 360;} else {tlen = len;};\n"
"    len -= tlen;\n"
"    do \n"
"\t{\n"
"      sum1 += data.charCodeAt(i);\n"
"      sum2 += sum1;\n"
"      i++;\n"
"     } while (--tlen);\n"
"     sum1 = (sum1 & 0xffff) + (sum1 >> 16);\n"
"     sum2 = (sum2 & 0xffff) + (sum2 >> 16);\n"
"  }\n"
"   /* Second reduction step to reduce sums to 16 bits */\n"
"   sum1 = (sum1 & 0xffff) + (sum1 >> 16);\n"
"   sum2 = (sum2 & 0xffff) + (sum2 >> 16);\n"
" \n"
"   return sum2 << 16 | sum1;\n"
"}\n"
"\n"
"  \n"
"function cellContents(number, ch)\n"
"{\n"
"    var answer;\n"
"    if(ch == \" \")\n"
"    {\n"
"      ch = \"&nbsp;\";\n"
"    }\n"
"    ch = \"<font face = \\\"courier\\\"><b>\" + ch + \"</b></font>\"; \n"
"    if(number == 0)\n"
"    {\n"
"      answer = \"<sup><font size = \\\"1\\\">&nbsp;&nbsp;</font></sup>\" + ch;\n"
"    }\n"
"    else if(number <= 9)\n"
"    {\n"
"      answer = \"<sup><font size =\\\"1\\\">\" + number + \"&nbsp;</font></sup>\" + ch;\n"
"    }\n"
"    else\n"
"    {\n"
"      answer = \"<sup><font size = \\\"1\\\">\" + number + \"</font></sup>\" +ch;\n"
"    }\n"
"    \n"
"    return answer;\n"
"}\n"
"\n"
"\n"
"  \n"
"function crossword(grid, width, height, checksum) {\n"
"    var i;\n"
"\n"
"    this.grid = grid;\n"
"    this.width = width;\n"
"    this.height = height;\n"
"    this.checksum = checksum;\n"
"    this.selx = 0;\n"
"    this.sely = 0;\n"
"    this.numbers = generatenumbers(grid, width, height);\n"
"    this.entered = \"\";\n"
"    for(i=0;i<width*height;i++)\n"
"    {\n"
"      if(grid[i] == \"X\")\n"
"      {\n"
"        this.entered += \" \"; \n"
"      }\n"
"      else\n"
"      {\n"
"         this.entered += \".\";\n"
"      }\n"
"    }\n"
"   \n"
"\n"
"    this.setUpTable = function() {\n"
"      var x;\n"
"      var y;\n"
"\n"
"      for(y=0;y<this.height;y++)\n"
"      {\n"
"        var row = document.getElementById(\'myCrossword\').insertRow(y);\n"
"        for(x=0;x<this.width;x++)\n"
"        {\n"
"          var cell = row.insertCell(x);\n"
"          if(this.grid[y*this.width+x] == \'X\')\n"
"          {\n"
"            cell.setAttribute(\"bgcolor\", \"#FFFFFF\");\n"
"            cell.innerHTML = cellContents(this.numbers[y*this.width+x], \" \");\n"
"          }\n"
"          else\n"
"          {\n"
"            cell.innerHTML = cellContents(0, \" \");\n"
"          }\n"
"        }\n"
"      }\n"
"      this.setSelected();\n"
"    }\n"
"\n"
"    this.setSelected = function(){\n"
"      var x=document.getElementById(\'myCrossword\').rows[this.sely].cells;\n"
"      if(this.grid[this.sely*this.width+this.selx] == \'X\')\n"
"      {\n"
"         x[this.selx].setAttribute(\"bgcolor\", \"#00FF00\");\n"
"      }\n"
"      else\n"
"      {\n"
"         x[this.selx].setAttribute(\"bgcolor\", \"#008000\");\n"
"      }\n"
"     \n"
"    }\n"
"\n"
"    this.unsetSelected = function() {\n"
"      var x=document.getElementById(\'myCrossword\').rows[this.sely].cells;\n"
"       if(this.grid[this.sely*this.width+this.selx] == \'X\')\n"
"       {\n"
"         x[this.selx].setAttribute(\"bgcolor\", \"#FFFFFF\"); \n"
"       }\n"
"       else\n"
"       {\n"
"         x[this.selx].setAttribute(\"bgcolor\", \"#000000\");\n"
"       }\n"
"\n"
"    }\n"
"\n"
"    this.setCell = function(keyCode) {\n"
"      var index = this.sely * this.width + this.selx;\n"
"      var x=document.getElementById(\'myCrossword\').rows[this.sely].cells;\n"
"\n"
"      if(this.grid[this.sely*this.width+this.selx] == \'X\')\n"
"      {\n"
"         x[this.selx].innerHTML = cellContents(this.numbers[this.sely*this.width+this.selx], String."
"fromCharCode(keyCode));\n"
"         var newsolution = this.entered.substr(0, index-1) +\n"
"           String.fromCharCode(keyCode) + this.entered.substr(index + 1);\n"
"         this.entered = newsolution;\n"
"      }\n"
"\n"
"    }\n"
"}\n"
"\n"
"  var cw;\n"
"\n"
"\n"
"function GetChar (event){\n"
"            var keyCode = (\'which\' in event) \? event.which : event.keyCode;\n"
"           // alert (\"The Unicode key code is: \" + keyCode);\n"
"\n"
"            if(keyCode == 37)\n"
"            {\n"
"               if(cw.selx > 0) \n"
"               { cw.unsetSelected();\n"
"                 cw.selx -= 1;\n"
"                 cw.setSelected();\n"
"               };\n"
"            }\n"
"            else if(keyCode == 39)\n"
"            {\n"
"               if(cw.selx < cw.width-1) \n"
"               {\n"
"                 cw.unsetSelected();\n"
"                 cw.selx += 1;\n"
"                 cw.setSelected();\n"
"               };\n"
"            }\n"
"            else if(keyCode == 38)\n"
"            {\n"
"               if(cw.sely > 0) \n"
"               {\n"
"                 cw.unsetSelected();\n"
"                 cw.sely -= 1;\n"
"                 cw.setSelected();\n"
"               };\n"
"               event.preventDefault();\n"
"\n"
"            }\n"
"            else if(keyCode == 40)\n"
"            {\n"
"               if(cw.sely < cw.height-1)\n"
"               {\n"
"                 cw.unsetSelected();\n"
"                 cw.sely += 1;\n"
"                 cw.setSelected();\n"
"               }\n"
"               event.preventDefault();\n"
"            }\n"
"            else if(keyCode >= 97 && keyCode <= 122)\n"
"            {\n"
"              cw.setCell(keyCode - 97 + 65);\n"
"            }\n"
"            else if(keyCode >= 65 && keyCode <= 90)\n"
"            {\n"
"              cw.setCell(keyCode);\n"
"            }\n"
"            \n"
"            \n"
"\n"
"        }\n";
