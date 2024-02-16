% dont use 147, because not very fine steps 
clear
close all

%% modify this
save_figs = 0;
run_addm = 1; % 0=glam, 1=addm
setting1 = 1;
setting2 = 3;

meas = {'d','theta','s'};
Conds = 2; % 1=enc, 2=comp

switch setting1    
    case 1
        valueTransf = ''
    case 2
        valueTransf = '05'
    case 3
        valueTransf = '2' 
    case 4
        valueTransf = '147'
    case 5
        valueTransf = '2B'
    case 6
        valueTransf = '2BP'
    case 7
        valueTransf = '05BP'
    case 8
        valueTransf = '05B'
    case 9
        valueTransf = '14705'
    case 10
        valueTransf = '05S'
    case 11
        valueTransf = '14701'
    case 13
        valueTransf = '14701db'
    case 14
        valueTransf = '147T'
    case 15
        valueTransf = 'E4'
    case 16
        valueTransf = 'Ep2'
    case 17
        valueTransf = '147E2'
    case 18
        valueTransf = '147Ep3'

end


run_paramComp = 2; % 1=swam, 2=scatter
run_recovery = 0; %recovery level: 1,2;
check_autocorr = 0; show_corr = 1;


run_perfDiff  = 0;
run_behTest = 0;

load_exclSubj20 = 1;

%meas = {'accGaze', 'acc_left','acc_right','acc_nonzero', 'RT'}; num_measure =length(meas);

show_diagRefLine = 0;
figfolder = 'figures';

%% modeling results

if Conds == 1
    trialType1 = 'cardinal';
    trialType2 = 'diagonal';
    condiText = 'Encoding';
    filePrefix = 'enc';
    x ='Fine';
    y ='Coarse';
    color_dots =[0 0.4 1; 0.5 0.8 1];

elseif Conds == 2
    trialType1 = 'dvHigh';
    trialType2 = 'dvLow';
    condiText = 'Comparison';
    filePrefix = 'comp';
    x ='Easy';
    y ='Hard';
    color_dots =[1 0.5 0.5; 1 0 0];
end

recType = '_rec';

if run_recovery ==1
    trialType2 = trialType1;
    titleTextRec = [condiText ': ' x];
    filenameRec = ['fig_rec_' filePrefix x(1)];

elseif run_recovery ==2
    trialType1 = trialType2;
    titleTextRec = [condiText ': ' y];
    filenameRec = ['fig_rec_' filePrefix y(1)];
else
    recType = '';
end

if load_exclSubj20
    exclType = '_exclSubj20';
end


if ~run_addm
    meas ={'gamma', 'v', 's'};
    filefolder = fullfile('glambox','results','estimates');
    file1 = ['tavares2017' trialType1 '_tau1' exclType '_largeSamples_estimates.csv'];
    file2 = ['tavares2017' trialType2 '_tau1' exclType '_largeSamples' recType '_estimates.csv'];
    suffix = '_aDDM';
else
    meas ={'theta', 'd', 's'};
    filefolder = fullfile('datasets','tavares2017');
    file1 = ['output_' filePrefix x(1) valueTransf];
    file2 = ['output_' filePrefix y(1) valueTransf];
    suffix = '';
end




g = readtable(fullfile(filefolder, file1));
r = readtable(fullfile(filefolder, file2));


switch setting2
        case 1
        valueTransf2 = ''
    case 2
        valueTransf2 = '05'
    case 3
        valueTransf2 = '2' 
    case 4
        valueTransf2 = '147'
    case 5
        valueTransf2 = '2B'
    case 6
        valueTransf2 = '2BP'
    case 7
        valueTransf2 = '05BP'
    case 8
        valueTransf2 = '05B'
    case 9
        valueTransf2 = '14705'
    case 10
        valueTransf2 = '05S'
    case 11
        valueTransf2 = '14701'
    case 13
        valueTransf2 = '14701db'
    case 14
        valueTransf2 = '147T'
    case 15
        valueTransf2 = 'E4'
    case 16
        valueTransf2 = 'Ep2'
    case 17
        valueTransf2 = '147E2'
    case 18
        valueTransf2 = '147Ep3'

end

file1 = ['output_' filePrefix x(1) valueTransf2];
file2 = ['output_' filePrefix y(1) valueTransf2];

g2 = readtable(fullfile(filefolder, file1));
r2 = readtable(fullfile(filefolder, file2));

for m=1:length(meas)
    subplot(1,3,m)
    plot(g2.(meas{m})./g.(meas{m}));
    title(meas{m})
    sgtitle('[2*values] vs. [1*values] (high value difference trials)' )
%    sgtitle([filePrefix x valueTransf2 '/' valueTransf] )
ylabel('ratio')
xlabel('subject id')
    ylim([0,3])


end

figure
for m=1:length(meas)
    subplot(1,3,m)
    plot(r2.(meas{m})./r.(meas{m}));
    title(meas{m})
        sgtitle('[2*values] vs. [1*values] (low value difference trials)' )

    % sgtitle([filePrefix y valueTransf2 '/' valueTransf] )
    ylabel('ratio')
xlabel('subject id')

ylim([0,3])
end

