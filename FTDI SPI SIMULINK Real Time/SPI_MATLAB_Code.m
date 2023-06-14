% Libname matches the name of the .dll

Libname = 'libmpsse';

% "Don't call loadlibrary if the library is already loaded into memory" (loadlibrary help)
% The second argument is the header file containing the list of functions from Libname
% Note that it's ONLY used for the list of functions.  Therefore, any #include statements should be
% removed, and any typedefs used in the function declarations must be typedef'd inside the h file.
% Because the library is already compiled in the dll, the #include statements are extraneous, and cause
% loadlibrary to fail as it attempts to preprocess the header file.

if ~libisloaded( Libname )
	[notfound,warnings] = loadlibrary( Libname, 'ftd2xx', 'addheader', 'libmpsse_spi');
end

% Quick view of other library functions
% libfunctionsview(Libname)

% This shouldn't be required.
% calllib(Libname,'Init_libMPSSE');

% This is how we define a pointer in matlab.  Required to match the parameter types in various functions.
pNumchannels = libpointer('uint32Ptr',255);
pNumBytesTransferred = libpointer('uint32Ptr',255);
pChannelHandle = libpointer('voidPtr',255);

% Get the number of SPI channels available.  If 1, we can talk to an FTDI chip.  If 0... not so much
calllib(Libname,'SPI_GetNumChannels',pNumchannels); pause(0.1);
sprintf('Channels Found: %d',get(pNumchannels,'value'))

% Connect to SPI channel 0.  Valid numbers are 0:(Numchannels-1).
calllib(Libname,'SPI_OpenChannel',0,pChannelHandle); pause(0.1);

% Define the channel configuration struct, and initialize the channel.
ChConfig.ClockRate = uint32(10e6); % Clock speed, Hz
ChConfig.LatencyTimer = uint8(2); % Users guide section 3.4, suggested value is 2-255 for all devices
ChConfig.configOptions = uint32(0); % Bit 1 is CPOL, bit 0 is CPHA.  Higher order bits configure the chip select.
calllib(Libname,'SPI_InitChannel',pChannelHandle,ChConfig); pause(0.1);

% Write to the SPI.
writebuffer = [5 3 2 3 6 5];
transfer_options = 0; % No chip select used.
calllib(Libname,'SPI_Write',pChannelHandle,writebuffer,length(writebuffer),pNumBytesTransferred,transfer_options);

% Clean up
calllib(Libname,'SPI_CloseChannel',pChannelHandle);

return