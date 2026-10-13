# Drops each standings table fastchess prints into its console: a dashed rule,
# a header line beginning "Rank Name", the rows, and the closing dashed rule.
# Matched by shape only, so no value in a table is read (DEC-108, S264).
function is_rule(s) { return s ~ /^----------+$/ }
{
  if (skip) {
    if (is_rule($0)) skip = 0
    next
  }
  if (held) {
    held = 0
    if ($0 ~ /^Rank Name/) { skip = 1; dropped++; next }
    print rule
  }
  if (is_rule($0)) { held = 1; rule = $0; next }
  print
}
END {
  if (held) print rule
  print dropped + 0 > "/dev/stderr"
}
