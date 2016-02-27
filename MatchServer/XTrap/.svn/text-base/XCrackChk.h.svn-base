/*////////////////////////////////////////////////////////////////////

Copyright (c) 2005 WiseLogic Corporation

Author:
 WiseLogic Oct, 12, 2005

*//////////////////////////////////////////////////////////////////////


/* Random Key Generator	*/
void	RandomKeyGenW(	char* strKeyValue);		/* Output Generated First Key	*/

/* Key Verifier	*/
int		XCrackCheckW(	char* strSerialKey,		/* Input Verify Key(From Client)*/
						char* strRandomValue,	/* Input Generated First Key	*/
						char* strHashValue);	/* Input Client Hash Value		*/

/* return value
	if the verification succeeds, the return value is none zero.
	if the verification fails( crack, virus, non-client, etc... ), the return value is zero.
*/
					