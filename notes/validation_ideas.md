## Validation ideas

- validate voice uniqueness for sequence objects (should it be unique per staff?)

### 2.0 completed

- confirm staff systems neither skip nor overlap measures.
- confirm that global.lyrics.lineMetadata and global.lyrics.lineOrder arrays match if both are provided.
- confirm event ids and note ids are unique
- confirm that event objects either have a duration or a full-measure indication but not both.
- confirm that events are either rests or have notes but not both.
- confirm that lyric line labels in event objects are valid values, if global.lyrics provided the valid values.
- confirm tied-to note ids exist and have the same part, pitch name, octave, and alteration as their counterparts.
- confirm events in beams are only specified once.
- confirm beam nesting level does not exceed number of flags on note values of enclosed events.
- confirm that the number of flags on events specified with beam hooks exceeds the beam nesting level.
- validate slurs (target, startNote, endNote)

### 1.0 completed

- confirm multimeasure rests span measures that exist
- confirm measure counts in each part (optionally: it may be permissible to have different numbers of measures per part. need to check)
- confirm layouts in "scores" have been defined in "layouts". This is required by the MNX doc and not enforced by the schema.
- confirm measures exist in staff systems
- confirm "part" and "labelref" values in staff and staff source objects contain valid values/part references (part ids)
- valid values for enums (some of these are enforced by the schema; we'll need to check):
    - "labelref" (not enforced)
    - "stem" (staff source stem direction) -- enforced by schema
    - "symbol" (group bracket) -- enforced by schema

### Questions and Suggestions

- more symbol styles for groups (in particular, deskBracket)
- since the staff object has a "labelref" property, should it not also have a "part" property?
- staff group objects need a labelref alternative (and "part" that it references).
- labelRef values are not enforced by the schema
- there needs to be a mechanism to specify automatic name/shortName for "labelref" values.
- should "systems" be a required propery of the page object?

