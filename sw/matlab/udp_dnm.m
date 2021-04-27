% Listen on all available Ethernet interfaces at local port 8000.
% Specify a LocalHost (host name or IP address) if known
u = udp('', 'LocalHost', '', 'LocalPort', 5012,'InputBufferSize',2048, 'Timeout', 2);
fopen(u);
% Receive a single UDP packe
while(1==1)
    packetData = fread(u)
end
% Clean up
fclose(u);
delete(u);
clear u