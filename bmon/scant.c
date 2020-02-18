scan_ip (s, b)
register char **s;
register char *b;
{
    scan_sp(s);
    if (**s++ !=  '['))
       return 0;
    for (i = 0; i < 3; i++)
	if ((scan_int (s, &b[i]) == 0) || (**s++ != '.'))
	    return 0;
    if ((scan_int (s, &b[i]) == 0) || (**s++ != ']'))
       return 0;
    return 1;
}
