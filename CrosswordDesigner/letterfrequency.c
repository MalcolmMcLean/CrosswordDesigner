
/*
*  Letter frequencies of words in dictionaries (not words
*   in a corpus of texts)
*  Return a percentage
*/
double letterfrequency(char ch)
{
	double answer = 0;

	switch (ch)
	{
	case 'E': 	answer = 11.1607; break; /* 56.88 */
	case 'A': 	answer = 8.4966; break;  /* 43.31 */
	case 'R': 	answer = 7.5809; break;  /* 38.64 */
	case 'I':	answer = 7.5448; break;  /* 38.45 */ 
	case 'O': 	answer = 7.1635; break;  /* 36.51 */
	case 'T': 	answer = 6.9509; break;  /* 35.43 */
	case 'N': 	answer = 6.6544; break;  /* 33.92 */
	case 'S': 	answer = 5.7351; break;  /* 29.23 */
	case 'L': 	answer = 5.4893; break;  /* 27.98 */
	case 'C': 	answer = 4.5388; break;  /* 23.13 */
	case 'U': 	answer = 3.6308; break;  /* 18.51 */
	case 'D': 	answer = 3.3844; break;  /* 17.25 */
	case 'P': 	answer = 3.1671; break;  /* 16.14 */
	case 'M': 	answer = 3.0129; break;  /* 15.36 */
	case 'H': 	answer = 3.0034; break;  /* 15.31 */
	case 'G':	answer = 2.4705; break;  /* 12.59 */
	case 'B': 	answer = 2.0720; break;  /* 10.56 */
	case 'F': 	answer = 1.8121; break;  /* 9.24 */
	case 'Y': 	answer = 1.7779; break;  /* 9.06 */
	case 'W':	answer = 1.2899; break;  /* 6.57 */
	case 'K': 	answer = 1.1016; break;  /* 5.61 */
	case 'V':	answer = 1.0074; break;  /* 5.13 */
	case 'X': 	answer = 0.2902; break;  /* 1.48 */
	case 'Z': 	answer = 0.2722; break;  /* 1.39 */
	case 'J': 	answer = 0.1965; break;  /* 1.00 */
	case 'Q':	answer = 0.1962; break;  /* (1) */
	}

	return answer;
}