/* Program som r�knar anv�ndarna i NiKom */

cnt=0
do x=0 to 500
	if UserInfo(x,'n')~=-1 then cnt=cnt+1
end

say cnt 'anv�ndare!'