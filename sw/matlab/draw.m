% this is the only change now, what used to be
%datafloat = double(fftdata);
%is no

% this below line takes "datafloat", which is a floating point
% representation of the raw data which we collected, and does a few things
% 1) data is windowed. (for when signal is not at bin center)
% 2) a small amount of noise is added.  This keeps any bins from being 0
% and messing up the plot by trying to take log(0).  It also raises the
% noise floor above the fixed pattern noise of our DDS, and keeps the plot
% "moving" so that you know that it is working (if everything stays the
% same is is very disconcerting!)
% 3) fft is taken
% 4) magnitude of that FFT is taken
% 5) result is plotted in dB, with freqs_khz on the axis
subplot(2,1,1);
plot(freqs,20*log10((abs(fft(((datafloat+randn(1,length(datafloat))).*hann(length(datafloat))'))))));
axis([0 fs -40 150]); 
xlabel('Frequency Hz')
subplot(2,1,2);
plot(real(datafloat));
hold on;
plot(imag(datafloat),'r');
hold off;
%toc;
drawnow;
    
%end; % while loop
    
    % reduce the buffer size so that packets are dropped until the next
    % data sequence is collected\