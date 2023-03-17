<?php
	plugin_header();
	$m      =   (strpos($PAGE, '_mono') > 0) ? 'm' : 's';
	$nf     =   (strpos($PAGE, '_x8') > 0) ? 8 : (
		        (strpos($PAGE, '_x4') > 0) ? 4 : 2
		        );
	$cc     =   ($m == 'm') ? 'mono' : 'stereo';
?>

<p>
	This plugin allows to perform A/B test around <?= $nf ?> <?= $cc ?> inputs with blind option.
</p>
<p>
	The blind test option allows to activate blind test around selected set of inputs.
	When the blind test is activated, all possible actions to are performed to make impossible to
	visually identify each input:
</p>
<ul>
	<li>All inputs become randomly shuffled and reordered.</li>
	<li>All controls that can provide necessary information about the channel become hidden.</li>
	<li>The rating values become changed to default values.</li>
</ul>

<p><b>Common controls:</b></p>
<ul>
	<? if ($nf > 2) {?>
	<li><b>Select all</b> - allows to select all inputs as candidates for blind test.</li>
	<li><b>Select none</b> - allows to deselect all inputs as candidates for blind test.</li>
	<? } ?>
	<li><b>Blind test</b> - toggles the blind test mode.</li>
	<li><b>Reset rate</b> - allows to simply reset all ratings values to default values.</li>
	<? if ($m == 'm') {?>
	<li><b>Mono</b> - converts stereo output to mono output for testing mono compatiblity.</li>
	<? } ?>
	<li><b>Mute</b> - mutes the output and deselects any channel as being A/B tested.</li>
</ul>

<p><b>Individual input controls:</b></p>
<ul>
	<li><b>User label</b> - custom user text to identify the input.</li>
	<li><b>In test</b> - allows to mark the input as selected for blind test.</li>
	<li><b>Rating</b> - the user rating that can be assigned to the corresponding input.</li>
	<li><b>Gain</b> - the makeup gain for the corresponding input.</li>
	<li><b>Active</b> - the button that activates corresponding input.</li>
</ul>
