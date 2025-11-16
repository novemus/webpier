param([string]$SlipwayModule = "")

Write-Host "Searching for WebPier tasks to remove...";

try {
    $tasksToRemove = Get-ScheduledTask -TaskPath "\WebPier\" -ErrorAction Stop;

    foreach ($task in $tasksToRemove) {
        if ($task.Author -ne "WebPier") { 
            continue;
        }
        if ($task.Actions[0].Execute -eq $SlipwayModule) { 
            continue;
        }

        try {
            Write-Host "Unregister task: $task.TaskName";
            Unregister-ScheduledTask -TaskName $task.TaskName -Confirm:$false;
        } catch {
            Write-Error "Failed to unregister task. Error: $_";
        }
    }
}
catch {
    Write-Warning "No scheduled WebPier tasks found";
    return;
} 
