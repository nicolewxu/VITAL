%% ADC value liveplotting & logging.
%
% Reads ADC values from the M2 microcontroller.
% saves those values to a csv.
fname = 'data_rec.csv'
sampling_rate = .01

%% VARIABLES

%----> for ***MAC***
M2USB = serial('/dev/tty.usbmodem411','Baudrate',9600);
% *** Check where your device is by opening terminal and entering the command:
% 'ls /dev/tty.usb' and tab-completing.
M2USB.InputBufferSize=100000;

fopen(M2USB);       % Open up the port to the M2 microcontroller.
flushinput(M2USB);  % Remove anything extranneous that may be in the buffer.

% Send initial packet to get first set of data from microcontroller
fwrite(M2USB,1);% Send a packet to the M2.
time = 0;       % Set the start time to 0.
i = 1;          % Set i to 1, the indexer.
tic;            % Start timer.


data = zeros(5,run_time*(1/sampling_rate))

try
    while toc<run_time

        %% Read in data and send confirmation packet
        m2_buffer = fgetl(M2USB);   % Load buffer
        fwrite(M2USB,1);            % Confirmation packet

        [m2_ADC1, remain] = strtok(m2_buffer);
        [m2_ADC2, remain2] = strtok(remain);
        [m2_ADC3, remain3] = strtok(remain2);
        [m2_ADC4] = strtok(remain3);

        time = toc; % Stamp the time the value was received

        data(i,:) = [toc, str2double(m2_ADC1), str2double(m2_ADC2), str2double(m2_ADC3), str2double(m2_ADC4)]

        i=i+1;  % Incrememnt indexer

    end
    csvwrite(fname, data)

catch ME
    ME.stack
    %Close serial object
    fclose(M2USB);
end
