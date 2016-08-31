%% Script to continuously read a real-time written text file and plot the values
% L. Tagliapietra - Aug. 2016

if(~exist('path_name', 'var'))
    disp('Variable ''path_name'' not found. Please set it to the directory where input files are written')
    return
end

%% Starting up

%Set how long should be the history of the plot
data_sampling_rate = 2000; % sps
plot_memory = 10.0; % seconds
plot_refresh = 300; % refresh the plots every plot_refresh input data
data_subsampling_ratio = 10; % subsampling ratio of input data
column_separator = '\t';
file_filter = '*.txt';
command_window_width = 100; %points

% find the last created file
filelist = dir([path_name, file_filter]);
[~, dr] = sort([filelist.datenum], 'descend');
newest_filename = filelist(dr(1)).name;

% open the newest created file
fid = fopen(strcat(path_name, newest_filename), 'r');

% Read the first line to retrieve column labels
labels = strsplit(fgetl(fid), column_separator);
n_channels = length(labels) - 1;

% Jump to the end of file
status = fseek(fid, 0, 'eof');

% initialize data structures
buffer_size = data_sampling_rate * plot_memory; % should be 10 sec
data = nan(buffer_size, length(labels));
index = 0;
count = 0;
nap_count = 0;
% Sleep a bit
pause(4/data_sampling_rate); % data are written at 2kHz, so wait for 4 samples

% set up the new figure
fig = figure();
pause(0.0005); % required to maximize the figure window
set(get(fig, 'JavaFrame'), 'Maximized', 1);
pos = zeros(1,n_channels);
h = zeros(1,n_channels);

for i=1:n_channels
    pos(i) = subaxis(ceil(n_channels/2),2,i, 'Spacing', 0.03, 'Padding', 0.01, 'Margin', 0.01);
    set(pos(i), 'DrawMode', 'fast');
    set(pos(i), 'NextPlot', 'replacechildren')
    h(i) = plot(0,0);
    set(h(i), 'EraseMode', 'none');
    %legend(pos(i), {labels{i+1}}, 'Interpreter', 'none', 'Location', 'NorthEast');
    set(pos(i), 'Title', text('String', labels(i+1), 'Interpreter', 'none'));
end

%% Real-time reading and plotting
while(ishandle(fig))
    where_I_was = ftell(fid);
    where_I_am = fseek(fid, where_I_was, 'cof');
    while ~feof(fid)
        line = fgetl(fid);
        if(line ~= -1)
            count = count + 1;
            if ( mod(count,data_subsampling_ratio) == 0 )
                if (index <= buffer_size)
                    index = index + 1;
                else
                    data(1,:) = [];
                end;
                data(index,:) = str2double(strsplit(line, column_separator));
            end
             if ( mod ( count, plot_refresh ) == 0 )
                for i=1:n_channels
                    set(pos(i), 'XLim', [data(index) - plot_memory, data(index,1)+1.5]  )
                    set(h(i), 'xdata', data(:,1));
                    set(h(i), 'ydata', data(:,i+1));
                    drawnow update;
                end;
             end
        end;
    end;

    pause(1/data_sampling_rate);
    fprintf('.');
    nap_count = nap_count +1;
    if nap_count > command_window_width
        nap_count = 0;
        fprintf('\n');
    end
end

%% Close and quit
fprintf('\n');
fid = fclose(fid);
